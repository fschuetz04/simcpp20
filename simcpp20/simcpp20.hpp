// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#pragma once

#include <cassert>    // assert
#include <coroutine>  // std::coroutine_handle, std::suspend_never
#include <cstdint>    // uint64_t
#include <functional> // std::function, std::greater
#include <memory>     // std::make_shared, std::shared_ptr
#include <optional>   // std::optional
#include <queue>      // std::priority_queue
#include <vector>     // std::vector

#include "event.hpp"
