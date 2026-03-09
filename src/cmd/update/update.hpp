#pragma once
#include <string>

namespace update {

enum class UpdateMode { Stable, Edge, Tag, Git };

// Main update function
void perform(UpdateMode mode, const std::string& gitTag = "");

// Helper to convert choice integer to UpdateMode
UpdateMode choiceToMode(int choice);

} // namespace update
