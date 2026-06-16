#pragma once

#include <type_traits>

struct TrivialType
{};

static_assert(std::is_trivially_copyable_v<TrivialType>);

struct NonTrivialType
{
    NonTrivialType() {}
    ~NonTrivialType() {}
};

static_assert(!std::is_trivially_copyable_v<NonTrivialType>);
