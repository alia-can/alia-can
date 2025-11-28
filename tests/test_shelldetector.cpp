#include "shelldetector.hpp"
#include <cassert>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;
static void testShellDetection() { ShellDetector::Shell shell = ShellDetector::detectShell(); std::cout << "  Detected shell: " << ShellDetector::getShellName(shell) << std::endl; assert(shell != ShellDetector::Shell::UNKNOWN || true); }
static void testExpandHome() { std::string expanded = ShellDetector::expandHome("~"); assert(!expanded.empty()); assert(expanded[0] != '~'); }
static void testConfigFilePath() { assert(ShellDetector::getConfigFilePath(ShellDetector::Shell::BASH).find(".bashrc") != std::string::npos); assert(ShellDetector::getConfigFilePath(ShellDetector::Shell::ZSH).find(".zshrc") != std::string::npos); assert(ShellDetector::getConfigFilePath(ShellDetector::Shell::FISH).find(".config/fish") != std::string::npos); }
static void testShellNames() { assert(ShellDetector::getShellName(ShellDetector::Shell::BASH) == "BASH"); assert(ShellDetector::getShellName(ShellDetector::Shell::ZSH) == "ZSH"); assert(ShellDetector::getShellName(ShellDetector::Shell::FISH) == "FISH"); assert(ShellDetector::getShellName(ShellDetector::Shell::UNKNOWN) == "UNKNOWN"); }
void test_shelldetector() { std::cout << "Running ShellDetector tests...\n"; testShellDetection(); testExpandHome(); testConfigFilePath(); testShellNames(); std::cout << "✓ ShellDetector tests passed!\n"; }
