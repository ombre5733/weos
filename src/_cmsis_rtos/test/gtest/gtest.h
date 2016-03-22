#ifndef GTEST_HPP
#define GTEST_HPP

#include <stdint.h>
#include <stdio.h>

// Use this annotation at the end of a struct/class definition to
// prevent the compiler from optimizing away instances that are never
// used.  This is useful when all interesting logic happens inside the
// c'tor and / or d'tor.  Example:
//
//   struct Foo {
//     Foo() { ... }
//   } GTEST_ATTRIBUTE_UNUSED_;
//
// Also use it after a variable or parameter declaration to tell the
// compiler the variable/parameter does not have to be used.
#if defined(__GNUC__) && !defined(COMPILER_ICC)
# define GTEST_ATTRIBUTE_UNUSED_ __attribute__ ((unused))
#else
# define GTEST_ATTRIBUTE_UNUSED_
#endif

// A macro to disallow operator=
// This should be used in the private: declarations for a class.
#define GTEST_DISALLOW_ASSIGN_(type)\
  void operator=(type const &)

// A macro to disallow copy constructor and operator=
// This should be used in the private: declarations for a class.
#define GTEST_DISALLOW_COPY_AND_ASSIGN_(type)\
  type(type const &);\
  GTEST_DISALLOW_ASSIGN_(type)

namespace testing
{
class TestInfo;
class Test;

namespace internal
{

typedef void (*SetUpTestCaseFunc)();
typedef void (*TearDownTestCaseFunc)();

// Defines the abstract factory interface that creates instances
// of a Test object.
class TestFactoryBase {
 public:
  virtual ~TestFactoryBase() {}

  // Creates a test instance to run. The instance is both created and destroyed
  // within TestInfo::Run()
  virtual Test* CreateTest() = 0;

 protected:
  TestFactoryBase() {}

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(TestFactoryBase);
};

// This class provides implementation of TeastFactoryBase interface.
// It is used in TEST and TEST_F macros.
template <class TestClass>
class TestFactoryImpl : public TestFactoryBase {
 public:
  virtual Test* CreateTest() { return new TestClass; }
};

TestInfo* MakeAndRegisterTestInfo(
    const char* test_case_name, const char* name,
    const char* type_param,
    const char* value_param,
    int /*TypeId*/ fixture_class_id,
    SetUpTestCaseFunc set_up_tc,
    TearDownTestCaseFunc tear_down_tc,
    TestFactoryBase* factory);

class UnitTest
{
public:
    void addTestInfo(TestInfo* info);
    int runAllTests();

    static UnitTest* instance();

private:
    UnitTest();
    TestInfo* m_first;

    GTEST_DISALLOW_COPY_AND_ASSIGN_(UnitTest);
};

} // namespace internal

#if 0
// A copyable object representing the result of a test part (i.e. an
// assertion or an explicit FAIL(), ADD_FAILURE(), or SUCCESS()).
//
// Don't inherit from TestPartResult as its destructor is not virtual.
class GTEST_API_ TestPartResult {
 public:
  // The possible outcomes of a test part (i.e. an assertion or an
  // explicit SUCCEED(), FAIL(), or ADD_FAILURE()).
  enum Type {
    kSuccess,          // Succeeded.
    kNonFatalFailure,  // Failed but the test can continue.
    kFatalFailure      // Failed and the test should be terminated.
  };

  // C'tor.  TestPartResult does NOT have a default constructor.
  // Always use this constructor (with parameters) to create a
  // TestPartResult object.
  TestPartResult(Type a_type,
                 const char* a_file_name,
                 int a_line_number,
                 const char* a_message)
      : type_(a_type),
        file_name_(a_file_name),
        line_number_(a_line_number),
        summary_(ExtractSummary(a_message)),
        message_(a_message) {
  }

  // Gets the outcome of the test part.
  Type type() const { return type_; }

  // Gets the name of the source file where the test part took place, or
  // NULL if it's unknown.
  const char* file_name() const { return file_name_.c_str(); }

  // Gets the line in the source file where the test part took place,
  // or -1 if it's unknown.
  int line_number() const { return line_number_; }

  // Gets the summary of the failure message.
  const char* summary() const { return summary_.c_str(); }

  // Gets the message associated with the test part.
  const char* message() const { return message_.c_str(); }

  // Returns true iff the test part passed.
  bool passed() const { return type_ == kSuccess; }

  // Returns true iff the test part failed.
  bool failed() const { return type_ != kSuccess; }

  // Returns true iff the test part non-fatally failed.
  bool nonfatally_failed() const { return type_ == kNonFatalFailure; }

  // Returns true iff the test part fatally failed.
  bool fatally_failed() const { return type_ == kFatalFailure; }
 private:
  Type type_;

  // Gets the summary of the failure message by omitting the stack
  // trace in it.
  static internal::String ExtractSummary(const char* message);

  // The name of the source file where the test part took place, or
  // NULL if the source file is unknown.
  internal::String file_name_;
  // The line in the source file where the test part took place, or -1
  // if the line number is unknown.
  int line_number_;
  internal::String summary_;  // The test failure summary.
  internal::String message_;  // The test failure message.
};

// The result of a single Test.  This includes a list of
// TestPartResults, a list of TestProperties, a count of how many
// death tests there are in the Test, and how much time it took to run
// the Test.
//
// TestResult is not copyable.
class TestResult {
 public:
  // Creates an empty TestResult.
  TestResult();

  // D'tor.  Do not inherit from TestResult.
  ~TestResult();

  // Gets the number of all test parts.  This is the sum of the number
  // of successful test parts and the number of failed test parts.
  int total_part_count() const;

  // Returns the number of the test properties.
  int test_property_count() const;

  // Returns true iff the test passed (i.e. no test part failed).
  bool Passed() const { return !Failed(); }

  // Returns true iff the test failed.
  bool Failed() const;

  // Returns true iff the test fatally failed.
  bool HasFatalFailure() const;

  // Returns true iff the test has a non-fatal failure.
  bool HasNonfatalFailure() const;

  // Returns the elapsed time, in milliseconds.
  TimeInMillis elapsed_time() const { return elapsed_time_; }

  // Returns the i-th test part result among all the results. i can range
  // from 0 to test_property_count() - 1. If i is not in that range, aborts
  // the program.
  const TestPartResult& GetTestPartResult(int i) const;

 private:
  friend class TestInfo;
  friend class UnitTest;
  friend class internal::DefaultGlobalTestPartResultReporter;
  friend class internal::TestResultAccessor;
  friend class internal::UnitTestImpl;

  // Gets the vector of TestPartResults.
  const std::vector<TestPartResult>& test_part_results() const {
    return test_part_results_;
  }

  // Sets the elapsed time.
  void set_elapsed_time(TimeInMillis elapsed) { elapsed_time_ = elapsed; }

  // Adds a test part result to the list.
  void AddTestPartResult(const TestPartResult& test_part_result);

  // Clears the test part results.
  void ClearTestPartResults();

  // Clears the object.
  void Clear();

  // Protects mutable state of the property vector and of owned
  // properties, whose values may be updated.
  internal::Mutex test_properites_mutex_;

  // The vector of TestPartResults
  std::vector<TestPartResult> test_part_results_;
  // The elapsed time, in milliseconds.
  TimeInMillis elapsed_time_;

  // We disallow copying TestResult.
  GTEST_DISALLOW_COPY_AND_ASSIGN_(TestResult);
};  // class TestResult
#endif

// A TestInfo object stores the following information about a test:
//
//   Test case name
//   Test name
//   Whether the test should be run
//   A function pointer that creates the test object when invoked
//   Test result
//
// The constructor of TestInfo registers itself with the UnitTest
// singleton such that the RUN_ALL_TESTS() macro knows which tests to
// run.
class TestInfo {
 public:
  // Destructs a TestInfo object.  This function is not virtual, so
  // don't inherit from TestInfo.
  ~TestInfo();

  // Returns the test case name.
  const char* test_case_name() const { return test_case_name_; }

  // Returns the test name.
  const char* name() const { return name_; }

  /*
  // Returns the name of the parameter type, or NULL if this is not a typed
  // or a type-parameterized test.
  const char* type_param() const {
    if (type_param_.get() != NULL)
      return type_param_->c_str();
    return NULL;
  }

  // Returns the text representation of the value parameter, or NULL if this
  // is not a value-parameterized test.
  const char* value_param() const {
    if (value_param_.get() != NULL)
      return value_param_->c_str();
    return NULL;
  }
  */

  // Returns the result of the test.
  //const TestResult* result() const { return &result_; }

 private:

  /*
  friend class Test;
  friend class TestCase;
  */
  friend class internal::UnitTest;
  friend TestInfo* internal::MakeAndRegisterTestInfo(
      const char* test_case_name, const char* name,
      const char* type_param,
      const char* value_param,
      int /*internal::TypeId*/ fixture_class_id,
      internal::SetUpTestCaseFunc set_up_tc,
      internal::TearDownTestCaseFunc tear_down_tc,
      internal::TestFactoryBase* factory);

  // Constructs a TestInfo object. The newly constructed instance assumes
  // ownership of the factory object.
  TestInfo(const char* test_case_name, const char* name,
           const char* a_type_param,
           const char* a_value_param,
           internal::TestFactoryBase* factory);

  // Creates the test object, runs it, records its result, and then
  // deletes it.
  void Run();

  /*
  static void ClearTestResult(TestInfo* test_info) {
    test_info->result_.Clear();
  }
  */

  // These fields are immutable properties of the test.
  const char* test_case_name_;     // Test case name
  const char* name_;               // Test name
  // Name of the parameter type, or NULL if this is not a typed or a
  // type-parameterized test.
  //const internal::scoped_ptr<const ::std::string> type_param_;
  // Text representation of the value parameter, or NULL if this is not a
  // value-parameterized test.
  //const internal::scoped_ptr<const ::std::string> value_param_;
  //const internal::TypeId fixture_class_id_;   // ID of the test fixture class
  internal::TestFactoryBase* const factory_;  // The factory that creates
                                              // the test object

  // This field is mutable and needs to be reset before running the
  // test for the second time.
  //TestResult result_;

  TestInfo* m_next;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(TestInfo);
};

class Test {
 public:
  friend class TestInfo;

  // The d'tor is virtual as we intend to inherit from Test.
  virtual ~Test() {}

  // Sets up the stuff shared by all tests in this test case.
  //
  // Google Test will call Foo::SetUpTestCase() before running the first
  // test in test case Foo.  Hence a sub-class can define its own
  // SetUpTestCase() method to shadow the one defined in the super
  // class.
  static void SetUpTestCase() {}

  // Tears down the stuff shared by all tests in this test case.
  //
  // Google Test will call Foo::TearDownTestCase() after running the last
  // test in test case Foo.  Hence a sub-class can define its own
  // TearDownTestCase() method to shadow the one defined in the super
  // class.
  static void TearDownTestCase() {}

  // Returns true iff the current test has a fatal failure.
  static bool HasFatalFailure();

  // Returns true iff the current test has a non-fatal failure.
  static bool HasNonfatalFailure();

  // Returns true iff the current test has a (either fatal or
  // non-fatal) failure.
  static bool HasFailure() { return HasFatalFailure() || HasNonfatalFailure(); }

 protected:
  // Creates a Test object.
  Test() {}

  // Sets up the test fixture.
  virtual void SetUp() {}

  // Tears down the test fixture.
  virtual void TearDown() {}

 private:
  // Returns true iff the current test has the same fixture class as
  // the first test in the current test case.
  static bool HasSameFixtureClass();

  // Runs the test after the test fixture has been set up.
  //
  // A sub-class must implement this to define the test logic.
  //
  // DO NOT OVERRIDE THIS FUNCTION DIRECTLY IN A USER PROGRAM.
  // Instead, use the TEST or TEST_F macro.
  virtual void TestBody() = 0;

  // Sets up, executes, and tears down the test.
  void Run();

  // Deletes self.  We deliberately pick an unusual name for this
  // internal method to avoid clashing with names used in user TESTs.
  void DeleteSelf_() { delete this; }

  // Often a user mis-spells SetUp() as Setup() and spends a long time
  // wondering why it is never called by Google Test.  The declaration of
  // the following method is solely for catching such an error at
  // compile time:
  //
  //   - The return type is deliberately chosen to be not void, so it
  //   will be a conflict if a user declares void Setup() in his test
  //   fixture.
  //
  //   - This method is private, so it will be another compiler error
  //   if a user calls it from his test fixture.
  //
  // DO NOT OVERRIDE THIS FUNCTION.
  //
  // If you see an error about overriding the following function or
  // about it being private, you have mis-spelled SetUp() as Setup().
  struct Setup_should_be_spelled_SetUp {};
  virtual Setup_should_be_spelled_SetUp* Setup() { return 0; }

  // We disallow copying Tests.
  GTEST_DISALLOW_COPY_AND_ASSIGN_(Test);
};

} // namespace testing

#define GTEST_TEST_CLASS_NAME_(test_case_name, test_name) \
  test_case_name##_##test_name##_Test

#define GTEST_TEST_(test_case_name, test_name, parent_class, parent_id)\
class GTEST_TEST_CLASS_NAME_(test_case_name, test_name) : public parent_class {\
 public:\
  GTEST_TEST_CLASS_NAME_(test_case_name, test_name)() {}\
 private:\
  virtual void TestBody();\
  static ::testing::TestInfo* const test_info_ GTEST_ATTRIBUTE_UNUSED_;\
  GTEST_DISALLOW_COPY_AND_ASSIGN_(\
      GTEST_TEST_CLASS_NAME_(test_case_name, test_name));\
};\
\
::testing::TestInfo* const GTEST_TEST_CLASS_NAME_(test_case_name, test_name)\
  ::test_info_ =\
    ::testing::internal::MakeAndRegisterTestInfo(\
        #test_case_name, #test_name, NULL, NULL, \
        (parent_id), \
        parent_class::SetUpTestCase, \
        parent_class::TearDownTestCase, \
        new ::testing::internal::TestFactoryImpl<\
            GTEST_TEST_CLASS_NAME_(test_case_name, test_name)>);\
void GTEST_TEST_CLASS_NAME_(test_case_name, test_name)::TestBody()




#define ASSERT_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        printf("Failure in file %s, line %d, function: %s\n", \
            __FILE__, __LINE__, __PRETTY_FUNCTION__); \
        return; \
    }

#define ASSERT_TRUE(expr) \
    if (!(expr)) { \
        printf("Failure in file %s, line %d, function: %s\n", \
            __FILE__, __LINE__, __PRETTY_FUNCTION__); \
        return; \
    }

#define ASSERT_FALSE(expr) \
    if ((expr)) { \
        printf("Failure in file %s, line %d, function: %s\n", \
            __FILE__, __LINE__, __PRETTY_FUNCTION__); \
        return; \
    }

// Defines a test.
//
// The first parameter is the name of the test case, and the second
// parameter is the name of the test within the test case.
//
// The convention is to end the test case name with "Test".  For
// example, a test case for the Foo class can be named FooTest.
//
// The user should put his test code between braces after using this
// macro.  Example:
//
//   TEST(FooTest, InitializesCorrectly) {
//     Foo foo;
//     EXPECT_TRUE(foo.StatusIsOK());
//   }

// Note that we call GetTestTypeId() instead of GetTypeId<
// ::testing::Test>() here to get the type ID of testing::Test.  This
// is to work around a suspected linker bug when using Google Test as
// a framework on Mac OS X.  The bug causes GetTypeId<
// ::testing::Test>() to return different values depending on whether
// the call is from the Google Test framework itself or from user test
// code.  GetTestTypeId() is guaranteed to always return the same
// value, as it always calls GetTypeId<>() from the Google Test
// framework.
#define TEST(test_case_name, test_name)\
  GTEST_TEST_(test_case_name, test_name, \
              ::testing::Test, 0)//::testing::internal::GetTestTypeId())

// Defines a test that uses a test fixture.
//
// The first parameter is the name of the test fixture class, which
// also doubles as the test case name.  The second parameter is the
// name of the test within the test case.
//
// A test fixture class must be declared earlier.  The user should put
// his test code between braces after using this macro.  Example:
//
//   class FooTest : public testing::Test {
//    protected:
//     virtual void SetUp() { b_.AddElement(3); }
//
//     Foo a_;
//     Foo b_;
//   };
//
//   TEST_F(FooTest, InitializesCorrectly) {
//     EXPECT_TRUE(a_.StatusIsOK());
//   }
//
//   TEST_F(FooTest, ReturnsElementCountCorrectly) {
//     EXPECT_EQ(0, a_.size());
//     EXPECT_EQ(1, b_.size());
//   }
#define TEST_F(test_fixture, test_name)\
  GTEST_TEST_(test_fixture, test_name, test_fixture, \
              ::testing::internal::GetTypeId<test_fixture>())

#define RUN_ALL_TESTS()\
    (::testing::internal::UnitTest::instance()->runAllTests())

#endif // GTEST_HPP
