#include "configfilehandler.hpp"
#include "backupmanager.hpp"
#include <cassert>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstdlib>

namespace fs = std::filesystem;

static std::string getTempTestFile() {
    char* tmpdir = getenv("TMPDIR");
    if (!tmpdir) tmpdir = const_cast<char*>("/tmp");
    return std::string(tmpdir) + "/alia-can-test-config";
}

static void cleanupTestFile() {
    std::string testFile = getTempTestFile();
    if (fs::exists(testFile)) fs::remove(testFile);

    for (const auto& entry : fs::directory_iterator(fs::path(testFile).parent_path())) {
        std::string name = entry.path().filename().string();
        if (name.find("alia-can-test-config") != std::string::npos) {
            try { fs::remove(entry.path()); } catch (...) {}
        }
    }
}

static void testLoadEmptyFile() {
    cleanupTestFile();
    ConfigFileHandler handler(getTempTestFile(), ShellDetector::Shell::BASH);
    assert(handler.loadAliases().empty());
}

static void testAddAlias() {
    cleanupTestFile();
    ConfigFileHandler handler(getTempTestFile(), ShellDetector::Shell::BASH);

    Alias a{"ll", "ls -la"};
    assert(handler.addAlias(a));

    auto aliases = handler.loadAliases();
    assert(aliases.size() == 1);
}

static void testRemoveAlias() {
    cleanupTestFile();
    ConfigFileHandler handler(getTempTestFile(), ShellDetector::Shell::BASH);

    handler.addAlias({"ll", "ls -la"});
    handler.addAlias({"gs", "git status"});

    handler.removeAlias("ll");
    auto aliases = handler.loadAliases();
    assert(aliases.size() == 1);
}

static void testMultipleAliases() {
    cleanupTestFile();
    ConfigFileHandler handler(getTempTestFile(), ShellDetector::Shell::ZSH);

    std::vector<Alias> list = {
        {"ll","ls -la"}, {"la","ls -A"}, {"l","ls -CF"}, {"gs","git status"}
    };

    for (auto& a : list) handler.addAlias(a);
    auto loaded = handler.loadAliases();
    assert(loaded.size() == list.size());
}

static void testValidationOnAdd() {
    cleanupTestFile();
    ConfigFileHandler handler(getTempTestFile(), ShellDetector::Shell::BASH);

    assert(!handler.addAlias({"bad name", "ls"}));
    assert(!handler.addAlias({"ll", ""}));
}

static void testBackupCreation() {
    cleanupTestFile();

    std::string file = getTempTestFile();
    ConfigFileHandler handler(file, ShellDetector::Shell::BASH);
    BackupManager backup(file);

    handler.addAlias({"ll", "ls -la"});

    std::string path = backup.createBackup();
    assert(!path.empty());
    assert(fs::exists(path));
}

static void testRestoreBackup() {
    cleanupTestFile();

    std::string file = getTempTestFile();
    ConfigFileHandler handler(file, ShellDetector::Shell::BASH);
    BackupManager backup(file);

    handler.addAlias({"ll", "ls -la"});
    std::string path = backup.createBackup();

    handler.addAlias({"gs", "git status"});
    assert(backup.restoreFromBackup(path));

    auto aliases = handler.loadAliases();
    assert(aliases.size() == 1);
}

void test_confighandler() {
    std::cout << "Running ConfigFileHandler tests...\n";

    testLoadEmptyFile();
    testAddAlias();
    testRemoveAlias();
    testMultipleAliases();
    testValidationOnAdd();
    testBackupCreation();
    testRestoreBackup();

    cleanupTestFile();

    std::cout << "✓ ConfigFileHandler tests passed!\n";
}
