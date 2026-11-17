#pragma once
#include "pch-debug.hpp"

#include <string>
#include <asp/iter.hpp>

// Geode 6 menghapus gd::string. Alias ini menambal semua pemakaian gd::string di source Globed.
namespace gd { using string = std::string; }

// prolly not worth?
// #include <asp/time.hpp>
// #include <core/net/NetworkManagerImpl.hpp>
// #include <Geode/Geode.hpp>
// #include <globed/core/SettingsManager.hpp>
// #include <globed/core/Core.hpp>
