#pragma once

#include <tinyfutures/futures/promise.hpp>

#include <vector>

namespace tiny::futures {

namespace detail {

// Your combinators:
// AllCombinator, FirstOfCombinator

template<template <typename> class Combinator, typename T>
auto Combine(std::vector<Future<T>>& futures) {
  return;  // Not implemented
}

}  // namespace detail

// All values / first error
template <typename T>
Future<std::vector<T>> All(std::vector<Future<T>> futures) {
  return MakeBrokenFuture<std::vector<T>>();  // Not implemented
  // Use detail::Combine
}

// First value or last error
template <typename T>
Future<T> FirstOf(std::vector<Future<T>> futures) {
  return MakeBrokenFuture<T>();  // Not implemented
  // Use detail::Combine
}

}  // namespace tiny::futures
