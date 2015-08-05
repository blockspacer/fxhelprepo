// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file defines utility functions for escaping strings.

// pathch linkangming 这个两个文件使用的是旧版的,因为新版的utf8字符串是不转义的,
// 导致乐库无法识别,为了兼容旧版的,把这个文件还原一下

#ifndef BASE_JSON_STRING_ESCAPE_H_
#define BASE_JSON_STRING_ESCAPE_H_
#pragma once

#include <string>

#include "base/base_export.h"
#include "base/strings/string16.h"

namespace base {

// Escape |str| appropriately for a JSON string literal, _appending_ the
// result to |dst|. This will create unicode escape sequences (\uXXXX).
// If |put_in_quotes| is true, the result will be surrounded in double quotes.
// The outputted literal, when interpreted by the browser, should result in a
// javascript string that is identical and the same length as the input |str|.
BASE_EXPORT void JsonDoubleQuote(const std::string& str,
                                 bool put_in_quotes,
                                 std::string* dst);

// Same as above, but always returns the result double quoted.
BASE_EXPORT std::string GetDoubleQuotedJson(const std::string& str);

BASE_EXPORT void JsonDoubleQuote(const string16& str,
                                 bool put_in_quotes,
                                 std::string* dst);

// Same as above, but always returns the result double quoted.
BASE_EXPORT std::string GetDoubleQuotedJson(const string16& str);

}  // namespace base

#endif  // BASE_JSON_STRING_ESCAPE_H_
