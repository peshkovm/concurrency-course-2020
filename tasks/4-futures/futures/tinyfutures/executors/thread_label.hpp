#pragma once

#include <string>

namespace tiny::executors {

using ThreadLabel = std::string;

void LabelThread(const ThreadLabel& label);

void ExpectThread(const ThreadLabel& label);

ThreadLabel GetThreadLabel();

}  // namespace tiny::executors
