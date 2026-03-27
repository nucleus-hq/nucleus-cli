#pragma once
#include <string>

namespace update {

enum class UpdateMode { Stable, Edge, Tag, Git, GitBranch };

// Main update function
void perform(UpdateMode mode, const std::string& inputTag = "", const std::string& branch = "");

// Helper to convert choice integer to UpdateMode
UpdateMode choiceToMode(int choice);

} // namespace update
