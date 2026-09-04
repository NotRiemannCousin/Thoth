#pragma once
#include <mutex>
#include <unordered_map>
#include <thread>

#include <Thoth/Http/Client/Definitions.hpp>
#include <Thoth/Http/_base.hpp>

namespace Thoth::Http {
    //! @brief Manages a pool of reusable HTTP sockets to optimize consecutive calls.
    //! @details
    //! ClientJanitor maintains a map from endpoint identifiers to vectors of live
    //! sockets, allowing the Client to reuse connections instead of repeatedly
    //! establishing new ones. A background janitor thread periodically sweeps sockets
    //! that have been idle for more than a configurable threshold (currently 1 minute).
    //!
    //! **Thread-safety:**
    //! - The internal `connectionPool` must be accessed while holding `poolMutex`.
    //! - Do not assume any locking is performed automatically; always lock `poolMutex`
    //!   before reading or writing `connectionPool`.
    //! - The janitor thread runs every ~30 seconds and will also lock `poolMutex` during
    //!   its sweep.
    //!
    //! **Lifetime:**
    //! - ClientJanitor is a singleton; access it via `Instance()`.
    //! - The background thread is stopped when the singleton is destroyed.
    //!
    //! **Use-cases:**
    //! - Advanced users can inspect `connectionPool` for statistics or debugging.
    //! - Most users should rely on the Client APIs directly and ignore this class.
    struct ClientJanitor {

        static ClientJanitor& Instance();
        void JanitorLoop(std::stop_token stopToken);

        std::mutex poolMutex;

        //! @brief Group multiple sockets connected to the same endpoint. Before using it lock the poolMutex
        //! to not break other threads.
        std::unordered_map<ClientConnectionKey, std::vector<std::shared_ptr<ClientConnection>>> connectionPool;
    private:
        ClientJanitor();

        std::jthread m_janitorThread;
    };
}