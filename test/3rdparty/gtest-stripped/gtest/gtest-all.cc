#include "gtest.h"


namespace testing
{

namespace internal
{

UnitTest::UnitTest()
    : m_first(0),
      m_currentTestInfo(0)
{
}

TestInfo* UnitTest::currentTestInfo() const
{
    return m_currentTestInfo;
}

void UnitTest::addTestInfo(TestInfo *info)
{
    if (!m_first)
        m_first = info;
    else
    {
        TestInfo* iter = m_first;
        while (iter->m_next)
            iter = iter->m_next;
        iter->m_next = info;
    }
}

bool UnitTest::runAllTests()
{
    bool failed = false;
    TestInfo* iter = m_first;
    while (iter)
    {
        printf("%s %s\n", iter->test_case_name(), iter->name());
        iter->clearTestResult();
        iter->Run();
        if (iter->result().failed())
            failed = true;

        iter = iter->m_next;
    }
    return !failed;
}

void UnitTest::setCurrentTestInfo(TestInfo* info)
{
    m_currentTestInfo = info;
}

UnitTest* UnitTest::instance()
{
    static UnitTest instance;
    return &instance;
}

// Creates a new TestInfo object and registers it with Google Test;
// returns the created object.
//
// Arguments:
//
//   test_case_name:   name of the test case
//   name:             name of the test
//   type_param:       the name of the test's type parameter, or NULL if
//                     this is not a typed or a type-parameterized test.
//   value_param:      text representation of the test's value parameter,
//                     or NULL if this is not a value-parameterized test.
//   fixture_class_id: ID of the test fixture class
//   set_up_tc:        pointer to the function that sets up the test case
//   tear_down_tc:     pointer to the function that tears down the test case
//   factory:          pointer to the factory that creates a test object.
//                     The newly created TestInfo instance will assume
//                     ownership of the factory object.
TestInfo* MakeAndRegisterTestInfo(const char* test_case_name, const char* name,
    const char* type_param,
    const char* value_param,
    int /*TypeId*/ fixture_class_id,
    SetUpTestCaseFunc set_up_tc,
    TearDownTestCaseFunc tear_down_tc,
    TestFactoryBase* factory)
{
  TestInfo* const test_info =
      new TestInfo(test_case_name, name, type_param, value_param, factory);
  UnitTest::instance()->addTestInfo(test_info);
  return test_info;
}

// Runs the given method and catches and reports C++ and/or SEH-style
// exceptions, if they are supported; returns the 0-value for type
// Result in case of an SEH exception.
template <class T, typename Result>
Result HandleExceptionsInMethodIfSupported(
    T* object, Result (T::*method)(), const char* location) {
  // NOTE: The user code can affect the way in which Google Test handles
  // exceptions by setting GTEST_FLAG(catch_exceptions), but only before
  // RUN_ALL_TESTS() starts. It is technically possible to check the flag
  // after the exception is caught and either report or re-throw the
  // exception based on the flag's value:
  //
  // try {
  //   // Perform the test method.
  // } catch (...) {
  //   if (GTEST_FLAG(catch_exceptions))
  //     // Report the exception as failure.
  //   else
  //     throw;  // Re-throws the original exception.
  // }
  //
  // However, the purpose of this flag is to allow the program to drop into
  // the debugger when the exception is thrown. On most platforms, once the
  // control enters the catch block, the exception origin information is
  // lost and the debugger will stop the program at the point of the
  // re-throw in this function -- instead of at the point of the original
  // throw statement in the code under test.  For this reason, we perform
  // the check early, sacrificing the ability to affect Google Test's
  // exception handling in the method where the exception is thrown.
  if (1) {// if (internal::GetUnitTestImpl()->catch_exceptions()) {
#if GTEST_HAS_EXCEPTIONS
    try {
      return (object->*method)();
    } catch (const GoogleTestFailureException&) {  // NOLINT
      // This exception doesn't originate in code under test. It makes no
      // sense to report it as a test failure.
      throw;
    } catch (const std::exception& e) {  // NOLINT
      internal::ReportFailureInUnknownLocation(
          TestPartResult::kFatalFailure,
          FormatCxxExceptionMessage(e.what(), location));
    } catch (...) {  // NOLINT
      internal::ReportFailureInUnknownLocation(
          TestPartResult::kFatalFailure,
          FormatCxxExceptionMessage(NULL, location));
    }
    return static_cast<Result>(0);
#else
    return (object->*method)();
#endif  // GTEST_HAS_EXCEPTIONS
  } else {
    return (object->*method)();
  }
}

} // namespace internal

// ----=====================================================================----
//     TestResult
// ----=====================================================================----
#if 0
// Creates an empty TestResult.
TestResult::TestResult()
    : elapsed_time_(0) {
}

// D'tor.
TestResult::~TestResult() {
}

// Returns the i-th test part result among all the results. i can
// range from 0 to total_part_count() - 1. If i is not in that range,
// aborts the program.
const TestPartResult& TestResult::GetTestPartResult(int i) const {
  if (i < 0 || i >= total_part_count())
    internal::posix::Abort();
  return test_part_results_.at(i);
}

// Clears the test part results.
void TestResult::ClearTestPartResults() {
  test_part_results_.clear();
}

// Adds a test part result to the list.
void TestResult::AddTestPartResult(const TestPartResult& test_part_result) {
  test_part_results_.push_back(test_part_result);
}

// Clears the object.
void TestResult::Clear() {
  test_part_results_.clear();
  elapsed_time_ = 0;
}

// Returns true iff the test failed.
bool TestResult::Failed() const {
  for (int i = 0; i < total_part_count(); ++i) {
    if (GetTestPartResult(i).failed())
      return true;
  }
  return false;
}

// Returns true iff the test part fatally failed.
static bool TestPartFatallyFailed(const TestPartResult& result) {
  return result.fatally_failed();
}

// Returns true iff the test fatally failed.
bool TestResult::HasFatalFailure() const {
  return CountIf(test_part_results_, TestPartFatallyFailed) > 0;
}

// Returns true iff the test part non-fatally failed.
static bool TestPartNonfatallyFailed(const TestPartResult& result) {
  return result.nonfatally_failed();
}

// Returns true iff the test has a non-fatal failure.
bool TestResult::HasNonfatalFailure() const {
  return CountIf(test_part_results_, TestPartNonfatallyFailed) > 0;
}

// Gets the number of all test parts.  This is the sum of the number
// of successful test parts and the number of failed test parts.
int TestResult::total_part_count() const {
  return static_cast<int>(test_part_results_.size());
}
#endif
// ----=====================================================================----
//     TestInfo
// ----=====================================================================----

// Constructs a TestInfo object. It assumes ownership of the test factory
// object.
TestInfo::TestInfo(const char* a_test_case_name,
                   const char* a_name,
                   const char* a_type_param,
                   const char* a_value_param,
                   internal::TestFactoryBase* factory)
    : test_case_name_(a_test_case_name),
      name_(a_name),
      //type_param_(a_type_param ? new std::string(a_type_param) : NULL),
      //value_param_(a_value_param ? new std::string(a_value_param) : NULL),
      factory_(factory),
      m_next(0)
{
}

// Destructs a TestInfo object.
TestInfo::~TestInfo()
{
    delete factory_;
}

void TestInfo::Run()
{

#if 0
  // Tells UnitTest where to store test result.
  internal::UnitTestImpl* const impl = internal::GetUnitTestImpl();
  impl->set_current_test_info(this);

  TestEventListener* listener = UnitTest::GetInstance()->listener();

  // Notifies the unit test event listeners that a test is about to start.
  listener->OnTestStart(*this);

  const TimeInMillis start = internal::GetTimeInMillis();

  // Creates the test object.
  Test* const test = internal::HandleExceptionsInMethodIfSupported(
      factory_, &internal::TestFactoryBase::CreateTest,
      "the test fixture's constructor");

  // Runs the test only if the test object was created and its
  // constructor didn't generate a fatal failure.
  if ((test != NULL) && !Test::HasFatalFailure()) {
    // This doesn't throw as all user code that can throw are wrapped into
    // exception handling code.
    test->Run();
  }

  // Deletes the test object.
  internal::HandleExceptionsInMethodIfSupported(
      test, &Test::DeleteSelf_, "the test fixture's destructor");

  result_.set_elapsed_time(internal::GetTimeInMillis() - start);

  // Notifies the unit test event listener that a test has just finished.
  listener->OnTestEnd(*this);

  // Tells UnitTest to stop associating assertion results to this
  // test.
  impl->set_current_test_info(NULL);
#endif

  internal::UnitTest::instance()->setCurrentTestInfo(this);

  Test* test = 0;
  try
  {
      test = factory_->CreateTest();
  }
  catch (...)
  {
      internal::UnitTest::instance()->currentTestInfo()->recordFailure(
                  "", -1);
      return;
  }

  test->Run();
  test->DeleteSelf_();

  internal::UnitTest::instance()->setCurrentTestInfo(0);
}

// ----=====================================================================----
//     Test
// ----=====================================================================----

// Runs the test and updates the test result.
void Test::Run()
{
    try
    {
        this->SetUp();
    }
    catch (...)
    {
        internal::UnitTest::instance()->currentTestInfo()->recordFailure(
                    "", -1);
        return;
    }

    try
    {
        this->TestBody();
    }
    catch (...)
    {
        internal::UnitTest::instance()->currentTestInfo()->recordFailure(
                    "", -1);
        return;
    }

    try
    {
        this->TearDown();
    }
    catch (...)
    {
        internal::UnitTest::instance()->currentTestInfo()->recordFailure(
                    "", -1);
        return;
    }
    /*
  if (!HasSameFixtureClass()) return;

  internal::HandleExceptionsInMethodIfSupported(this, &Test::SetUp, "SetUp()");
  // We will run the test only if SetUp() was successful.
  if (!HasFatalFailure()) {
    internal::HandleExceptionsInMethodIfSupported(
        this, &Test::TestBody, "the test body");
  }

  // However, we want to clean up as much as possible.  Hence we will
  // always call TearDown(), even if SetUp() or the test body has
  // failed.
  internal::HandleExceptionsInMethodIfSupported(
      this, &Test::TearDown, "TearDown()");
      */
}

} // namespace testing
