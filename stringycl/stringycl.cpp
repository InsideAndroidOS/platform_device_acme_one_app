/*
 * Copyright (C) 2020 Larry Schiefer and Blake Meike
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <hidl/HidlSupport.h>
#include <vendor/acme/one/stringy/1.0/IStringy.h>
#include <vendor/acme/one/stringy/1.0/types.h>
#include <utils/StrongPointer.h>
#include <getopt.h>

using vendor::acme::one::stringy::V1_0::IStringy;
using vendor::acme::one::stringy::V1_0::StringySummary;
using android::sp;

using namespace android::hardware;

static void reverseCb(const hidl_string& outputText) {
    printf("[reverse] %s\n", outputText.c_str());
}

static void splitCb(const hidl_vec<int8_t>& outputChars) {
    printf("[split] total size: %lu\n", outputChars.size());
    printf("[split] characters: ");
    for (int i = 0; i < outputChars.size(); i++) {
        printf("%c ", outputChars[i]);
    }

    printf("\n");
}

static void summaryCb(const StringySummary& summary) {
    printf("[summary] count: %u, hash: %8.8X\n",
           summary.charCount,
           summary.hash);
}

void printUsage(char *name) {
    printf("Usage: %s [-rhsS] input_string\n", name);
    printf("\tr:  Reverse the input string\n");
    printf("\th:  Hash the input string\n");
    printf("\ts:  Split the input string\n");
    printf("\tS:  Print the summary of the input string\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return -1;
    }

    bool doReverse = false;
    bool doHash = false;
    bool doSplit = false;
    bool doSummary = false;
    bool doDebug = false;
    int  currentOpt;

    //  Parse the command line options, determine what to do
    while ((currentOpt = getopt(argc, argv, "rhsS")) != -1) {
        switch (currentOpt) {
            case 'r':
                doReverse = true;
                break;
            case 'h':
                doHash = true;
                break;
            case 's':
                doSplit = true;
                break;
            case 'S':
                doSummary = true;
                break;
            case '?':
                if (isprint(optopt)) {
                    fprintf(stderr,
                            "Unknown option: '%c'\n",
                            optopt);
                } else {
                    fprintf(stderr,
                            "Unknown option character: '\\x%X'\n",
                            optopt);
                }

                printUsage(argv[0]);
                return -1;

            default:
                abort();
        }
    }

    if (!doReverse && !doHash && !doSplit && !doSummary) {
        doDebug = true;
    }

    if (optind >= argc) {
        fprintf(stderr, "No input text provided\n");
        printUsage(argv[0]);
        return -1;
    }

    const hidl_string inputString(argv[optind]);

    //  Get the HIDL to use (IStringy)
    sp<IStringy>  client = IStringy::getService();
    if (client == nullptr) {
        fprintf(stderr, "Unable to get stringy service interface\n");
        return -2;
    }

    if (doReverse) {
        client->reverse(inputString, reverseCb);
    }

    if (doHash) {
        Return<uint32_t> retHash = client->hash(inputString);
        if (!retHash.isOk()) {
            fprintf(stderr,
                    "Unable to hash input string (%s). Err: %s\n",
                    inputString.c_str(),
                    retHash.description().c_str());
            return -1;
        }

        printf("[hash] %8.8X\n", static_cast<uint32_t>(retHash));
    }

    if (doSplit) {
        client->split(inputString, splitCb);
    }

    if (doSummary) {
        client->summarize(inputString, summaryCb);
    }

    return 0;
}
