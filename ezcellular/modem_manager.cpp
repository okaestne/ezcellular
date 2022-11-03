/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later
*/
#include "modem_manager.h"

#include <algorithm> // std::remove_if
#include <map>       // std::map
#include <string>    // std::string
#include <utility>   // std::pair, std::move

#include "dbus_constants.h"
#include "exception.h"

namespace ezcellular {

/**
 * @brief Helper class to access the ModemManager D-Bus object.
 *
 * This class implements the D-Bus ObjectManager proxy interface.
 * The ModemManager class uses it to conveniently access the GetManagedObjects() method
 * and to be notified when new Modems are added and removed.
 * For this purpose, this class is used instead of the generic sdbus::IProxy interface.
 *
 * Adapted from:
 * https://github.com/Kistler-Group/sdbus-cpp/blob/master/examples/org.freedesktop.DBus.ObjectManager/obj-manager-client.cpp
 */
class ModemManagerOMProxy final : public sdbus::ProxyInterfaces<sdbus::ObjectManager_proxy> {
public:
    /**
     * @brief Constructor. Only to be invoked through the ModemManager class.
     * @param conn the D-Bus connection to use
     */
    explicit ModemManagerOMProxy(std::shared_ptr<sdbus::IConnection> conn)
        : ProxyInterfaces{*conn, DBus::MM_BUS_NAME, DBus::MM_OBJ_MODEMMANAGER}, conn_{std::move(conn)} {
        registerProxy();
        handleExisting();
    }

    // NOLINTBEGIN(*-trailing-return-type)
    ModemManagerOMProxy(const ModemManagerOMProxy&) = delete;
    ModemManagerOMProxy& operator=(const ModemManagerOMProxy&) = delete;
    ModemManagerOMProxy(ModemManagerOMProxy&&) = delete;
    ModemManagerOMProxy& operator=(ModemManagerOMProxy&&) = delete;
    // NOLINTEND(*-trailing-return-type)

    virtual ~ModemManagerOMProxy() {
        unregisterProxy();
    }

    /**
     * @brief Returns a (non-owning) reference to all Modems present
     */
    auto getModems() -> std::vector<Modem>& {
        return modems_;
    }

    /**
     * @brief Register to be notified when a modem becomes available
     *
     * @param imei the modem's IMEI or ANY_IMEI
     * @note will cancel waiting for other modems
     * @return std::future<Modem>
     */

    auto await_modem(std::string imei) {
        if (awaited_modem_) {
            awaited_modem_->second.set_exception(
                std::make_exception_ptr(ModemManagerException{"Cancelled, awaiting other modem now."}));
        }
        awaited_modem_ = std::make_pair<std::string, std::promise<Modem>>(std::move(imei), {});
        return awaited_modem_->second.get_future();
    }

private:
    std::shared_ptr<sdbus::IConnection> conn_;
    std::vector<Modem> modems_;
    std::optional<std::pair<std::string, std::promise<Modem>>> awaited_modem_;

    void handleExisting() {
        auto managed_objs = GetManagedObjects();
        for (const auto& [path, ifacesAndProps] : managed_objs) {  // structured binding (C++17)
            onInterfacesAdded(path, ifacesAndProps);
        }
    }

    // called, if a new modem is added
    void onInterfacesAdded(const sdbus::ObjectPath& objectPath,
                           [[maybe_unused]] const std::map<std::string, std::map<std::string, sdbus::Variant>>& interfacesAndProperties) override {
        Modem new_modem{conn_, objectPath};  // private ctor, but friend class

        if (awaited_modem_) {
            auto awaited_imei = awaited_modem_->first;
            auto new_imei = new_modem.imei();

            if (awaited_imei == ANY_IMEI || awaited_imei == new_imei) {
                awaited_modem_->second.set_value(new_modem);  // fulfill future
                awaited_modem_.reset(); // make the std::optional empty
            }
        }

        modems_.push_back(std::move(new_modem));  // emplace_back() would not be able to access the private ctor
    }

    void onInterfacesRemoved(const sdbus::ObjectPath& objectPath,
                             [[maybe_unused]] const std::vector<std::string>& interfaces) override {
        (void) std::remove_if(modems_.begin(), modems_.end(), [&](const Modem& m) {
            return m.dbus_proxy_->getObjectPath() == objectPath;
        });
    }
};

ModemManager::ModemManager()
    : conn_{sdbus::createSystemBusConnection()} {

    try {
        mm_proxy_ = std::make_unique<ModemManagerOMProxy>(conn_);
    } catch (const sdbus::Error&) {
        throw ModemManagerException("Failed to connect to ModemManager D-Bus API, is ModemManager running?");
    }

    // process event loop on separate thread
    conn_->enterEventLoopAsync();
}

ModemManager::~ModemManager() = default;

auto ModemManager::modems_available() const -> bool {
    return mm_proxy_->getModems().empty();
}

auto ModemManager::any_modem() const -> std::optional<Modem> {
    auto modems = mm_proxy_->getModems();
    if (modems.empty()) {
        return {}; // empty optional
    }
    return modems.at(0); // implicit copy
}

auto ModemManager::await_modem(std::string imei) const -> std::future<Modem> {
    return mm_proxy_->await_modem(std::move(imei));
}

auto ModemManager::available_modems() const -> std::vector<Modem> {
    return mm_proxy_->getModems(); // implicit copy
}

auto ModemManager::reset_modem(Modem& modem) const -> Modem {
    auto imei = modem.imei(); // get the IMEI to identify the restarted modem

    // register promise
    auto fut = await_modem(std::move(imei));
    // perform reset
    modem.reset();
    // wait until restarted
    fut.wait();
    return fut.get();
}

auto ModemManager::version() const -> std::string {
    auto proxy = sdbus::createProxy(*conn_, DBus::MM_BUS_NAME, DBus::MM_OBJ_MODEMMANAGER);
    return proxy->getProperty("Version").onInterface(DBus::MM_IF_MODEMMANAGER);
}

} // namespace ezcellular
