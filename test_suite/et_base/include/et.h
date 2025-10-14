//============================================================================
// ET: embedded test (super-simple embedded testing framework)
// GitHub: https://github.com/QuantumLeaps/Embedded-Test
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, <state-machine.com>.
//
// SPDX-License-Identifier: MIT
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//============================================================================
#ifndef ET_H_
#define ET_H_

// Embedded Test (ET) version
#define ET_VERSION "2.0.1"
#define TEST_DATA_SECTION   __attribute__((section(".et_group"), used))
#define ET_FIRST_GROUP      ((et_tc_group_t *)et_group_start)
#define ET_GROUP_COUNT      ((et_group_end - et_group_start)/sizeof(et_tc_group_t))

//! macro to define a test group
#define TEST_GROUP(mod_, id_, name_, setup_, clean_) \
    void ET_run_##mod_##id_(void); \
    et_tc_group_t ET_group_##mod_##id_ TEST_DATA_SECTION = { \
        .name = name_, \
        .op_func = { \
            .entry = ET_run_##mod_##id_, \
            .setup = setup_, \
            .clean = clean_, \
        }, \
    }; \
    void ET_run_##mod_##id_(void)

// macro to start a new test
#define TEST(title_) \
    if (ET_test_(title_, 0))

// macro to skip a test
#define SKIP_TEST(title_) \
    if (ET_test_(title_, 1))

//! macro to verify a test expectation
#define VERIFY(cond_) \
    ((cond_) ? (void)0 : ET_fail(#cond_, et_curr_group->name, __LINE__))

#define VERIFY_ASSERT(module_, label_) \
    ET_verify_assert_((module_), (label_))

//! macro to force a failure of a test
#define FAIL(note_) \
    (ET_fail(note_, &ET_group_[0], __LINE__))

#ifndef ARRAY_NELEM
//! convenience macro to provide the number of elements in the array a_
#define ARRAY_NELEM(a_)  (sizeof(a_) / sizeof((a_)[0]))
#endif // ARRAY_NELEM

#define ET_GROUP_MAX        10

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*et_op_func_t)(void);

typedef struct __et_group_op_t {
    et_op_func_t entry;
    et_op_func_t setup;         // be called when test group start
    et_op_func_t clean;         // be called when test group end
} et_group_op_t;

typedef struct __et_tc_group_t {
    char *name;
    et_group_op_t op_func;
} et_tc_group_t;

typedef enum {
    ET_MODULE_SEM = 0,
    ET_MODULE_EVENT,
    // can not add anything enum item behand ET_GROUP_MAX
    ET_MODULE_MAX,
} et_module_t;

// callback functions to be implemented in the ET board support packages
void ET_onInit(int argc, char *argv[]);
void ET_onPrintChar(char const ch);
void ET_onExit(int err);

// public helpers
void ET_fail(char const *cond, char const *group, int line);
void ET_expect_assert(char const *module, int label);
void ET_verify_assert_(char const *module, int label);

// private helpers
void ET_run_(void);
int  ET_test_(char const *title, int skip);
// extern char const ET_group_[];

extern unsigned int _et_group_start;
extern unsigned int _et_group_end;
extern et_tc_group_t *et_curr_group;

#ifdef __cplusplus
}
#endif

#endif // ET_H_
