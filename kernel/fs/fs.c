/*
Copyright 2021 Kris Foster

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdlib.h>
#include <string.h>
#include <machine.h>

#include "fs.h"

#include "../commands.h"

#include "../process.h"
#include "../bin/sh/sh.h"

void init_filesystem() {
    // Make sure this directory is empty
    for (int i=0;i<MAX_FILES;i++) {
        root.files[i].name = NULL;
    }
    fs.root = &root;     // Set up the location of "/" aka root

    // Let's dump some uh "files" into it!
    create_file("uptime",       FILE, (void *)uptime);
    create_file("ps",           FILE, (void *)ps);
    create_file("reboot",       FILE, (void *)reboot);
    create_file("who",          FILE, (void *)who);
    create_file("blinkd",       FILE, (void *)blinkd);
    create_file("memplay",      FILE, (void *)memplay);
    create_file("syscaller",    FILE, (void *)syscaller);
    create_file("dir",          FILE, (void *)dir);
    create_file("kill",         FILE, (void *)killer);
    create_file("argo",         FILE, (void *)argo);
    create_file("whoami",       FILE, (void *)whoami);
    create_file("sh",           FILE, (void *)sh);
}

void create_file(char *name, uint8_t type, void (*inode_start)()) {
    inode_t inode = {
        .start = inode_start,
    };
    // Find a slot in the directory for our file
    for (int i=0;i<MAX_FILES;i++) {
        if (fs.root->files[i].name == NULL) {
            fs.root->files[i].name = name;
            fs.root->files[i].type = type;
            fs.root->files[i].inode = inode;
            break;
        }
        // Oh no! Too many files, didn't add anything
    }
}

void delete_file(char *name) {

}

void list_directory() {

}
