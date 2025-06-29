#include "gtest/gtest.h"

#include "all-headers.hpp"


namespace {
	/*namespace memento {
		using namespace ::pattern::behavioral::memento;

		TEST(MementoTest, MementoMain) {
			MyOriginator my_originator{};
			my_originator.state_.a = 2;
			my_originator.state_.b = 3;
			Memento<MyOriginator, MyMementoState> my_memento{ my_originator.CreateMementoByValue() };

			my_originator.state_.a = 0;
			my_originator.state_.b = 0;
			my_originator.RestoreByCopy(my_memento);
			EXPECT_EQ(my_originator.state_.a, 2) << "RestoreByCopy failed";
			EXPECT_EQ(my_originator.state_.b, 3);

			my_originator.state_.a = 0;
			my_originator.state_.b = 0;
			my_originator.RestoreByMove(std::move(my_memento));
			EXPECT_EQ(my_originator.state_.a, 2);
			EXPECT_EQ(my_originator.state_.b, 3);
		};
	}*/


	namespace common {
        using namespace ::common;

//================ApplyObjMemFn============================================

        struct TestClass {
            void Func1(int a, double b) {
                a_ = a;
            }

            void Func2(int a, double b) const {
            }

            bool Func3() { return true; }

            double Func4(int a, double b) const {
                return a + b;
            }


            int a_{};
        };

        struct TestTempl {
            template<typename MemFnPtrType, typename ClassType>
            void Func1(MemFnPtrType ClassType::* mem_fn) {

            //template<typename ReturnType, typename ObjectType, typename... ArgsTypes>
            //void Func1(ReturnType (ObjectType::* mem_fn)(ArgsTypes...)) {
                int a = 1;
                //MemFnPtrType b;

                MemFnPtrTrait<decltype(mem_fn)> traits{};
            }
        };
        //common::MemFnPtrTraitsImpl<void __cdecl(int, double)>	{...}	common::MemFnPtrTraitsImpl<void __cdecl(int, double)>


        TEST(MemberFunctionTests, GeneralTest) {
           /* TestTempl b{};
            b.Func1(&TestClass::Func1);*/


            TestClass a{};
            InvokeMethod(&TestClass::Func1, a, 1, 2.0);
            InvokeMethod(&TestClass::Func2, a, 1, 2.0);
            InvokeMethod(&TestClass::Func3, a);
            InvokeMethod(&TestClass::Func4, a, 1, 2.0);

            std::shared_ptr<TestClass> shared{ std::make_shared<TestClass>() };
            InvokeMethodByPtr(&TestClass::Func1, shared, 1, 2.0);
            InvokeMethodByPtr(&TestClass::Func2, shared.get(), 1, 2.0);
            InvokeMethodByPtr(&TestClass::Func2, &a, 1, 2.0);
            //ApplyPtrMemFn(&A::Func2, nullptr, 1, 2.0);

            std::weak_ptr<TestClass> weak(shared);
            InvokeMethodByPtr(&TestClass::Func1, weak, 1, 2.0);
            InvokeMethodByPtr(&TestClass::Func2, weak, 1, 2.0);
        }

        TEST(MemberFunctionTests, CallExistingMethodWithReturnValue) {
            TestClass instance;
            auto result = InvokeMethod(&TestClass::Func4, instance, 3, 4.0);
            EXPECT_TRUE(std::get<0>(result)); // второй элемент должен быть true, если вызов прошел успешно
            EXPECT_EQ(std::get<1>(result), 7); // проверяем, что результат равен ожидаемой сумме
        }

        TEST(MemberFunctionTests, CallVoidMethod) {
            TestClass instance;
            auto result = InvokeMethod(&TestClass::Func2, instance, 3, 4);
            EXPECT_TRUE(std::get<0>(result)); // вызов прошел успешно
        }

        TEST(MemberFunctionTests, NonexistentMethod) {
            TestClass instance;
            int (*invalidFn)(int, int) = nullptr; // поддельный указатель на несуществующий метод
            //static_assert

            //auto result = ApplyObjMemFn(invalidFn, instance, 3, 4); // compile error
            //EXPECT_FALSE(std::get<0>(result)); // ожидание неудачи
        }

        TEST(MemberFunctionTests, NullptrMethod) {
            TestClass instance;
            //static_assert

            //auto result = ApplyObjMemFn(nullptr, instance);
            //EXPECT_FALSE(std::get<0>(result)); // ожидание неудачи
        }

        TEST(MemberFunctionTests, InvalidArgumentsCount) {
            TestClass instance;
            //Compile error

            //auto result = ApplyObjMemFn(&TestClass::Func1, instance); // неправильный набор аргументов
            //EXPECT_FALSE(std::get<0>(result)); // ожидание неудачи
        }


//============InvokeMemFn===============================================================

        class MyTestClass {
        public:
            int add(int a, int b) const {
                return a + b;
            }

            double multiply(double x, double y) {
                return x * y;
            }
        };

        //TEST(InvokeMemFnTests, AddMethod) {
        //    MyTestClass obj;
        //    auto result = InvokeMemFn(&MyTestClass::add, obj, 3, 7);  // объект передан по значению
        //    ASSERT_EQ(result(), 10);  // Проверяем возвращаемое значение
        //}

        //TEST(InvokeMemFnTests, MultiplyMethod) {
        //    MyTestClass obj;
        //    auto result = InvokeMemFn(&MyTestClass::multiply, obj, 3.5, 2.0);  // объект передан по значению
        //    ASSERT_DOUBLE_EQ(result(), 7.0);  // Проверка результата умножения
        //}

        //TEST(InvokeMemFnTests, RValueObject) {
        //    MyTestClass obj;
        //    auto result = InvokeMemFn(&MyTestClass::add, move(obj), 5, 8);  // передаем временный объект
        //    ASSERT_EQ(result(), 13);  // проверка корректности передачи rvalue
        //}

        //TEST(InvokeMemFnTests, StaticAssertionsFailures) {
        //    EXPECT_DEATH(
        //        InvokeMemFn(static_cast<int(MyTestClass::*)(int)>(nullptr), nullptr, 5),
        //        "");  // проверь статические проверки компилятора

        //    EXPECT_DEATH(
        //        InvokeMemFn(&MyTestClass::add, 5, 3, 7),
        //        "");  // передача необъекта должна вызвать ошибку

        //    EXPECT_DEATH(
        //        InvokeMemFn(nullptr, MyTestClass{}, 3, 7),
        //        "");  // вызов с нулевым указателем метода тоже запрещён
        //}

        //TEST(InvokeMemFnTests, NonexistentMethod) {
        //    using NotExistentFn = decltype(&MyTestClass::nonexistent_method);
        //    constexpr bool exists = HasMemberFn_v<NotExistentFn>;
        //    ASSERT_FALSE(exists);  // метод не существует, ожидаем false
        //}

//========================================================================================



		namespace thread {
			using namespace ::common::thread;

		} // !namespace thread

	} // !namespace common


    namespace error {
        using namespace ::error;

        TEST(ErrorTest, ErrorClass) {
            Error<ErrorInfoDetailed> my_error{};

            //EXPECT_EQ(GlobalVarsType::get_variable<GlobalVariablesEnum::a>(), -33);
            //EXPECT_EQ(GlobalVarsType::get_variable<GlobalVariablesEnum::b>(), 66.0);
        }

        TEST(ErrorTest, ExceptionClass) {
            GeneralException my_exception{ std::exception{}, ErrorInfoGeneral{FILE_N_LINE, "Hello World"} };

            //EXPECT_EQ(GlobalVarsType::get_variable<GlobalVariablesEnum::a>(), -33);
            //EXPECT_EQ(GlobalVarsType::get_variable<GlobalVariablesEnum::b>(), 66.0);
        }

    } // !namespace error


    /** https://google.github.io/googletest/primer.html */
    namespace help {
        // Info
        // Test Suite, Test Case
        // TEST(ObjectPoolTest, ObjectPoolClass) {}
        // Fixture class for common object for some Test Cases

        struct Foo {
            int a{};
        };

        // The fixture for testing class Foo.
        class FooTest : public testing::Test {
        protected:
            // You can remove any or all of the following functions if their bodies would
            // be empty.

            FooTest() {
                // You can do set-up work for each test here.
            }

            ~FooTest() override {
                // You can do clean-up work that doesn't throw exceptions here.
            }

            // If the constructor and destructor are not enough for setting up
            // and cleaning up each test, you can define the following methods:

            void SetUp() override {
                // Code here will be called immediately after the constructor (right
                // before each test).
            }

            void TearDown() override {
                // Code here will be called immediately after each test (right
                // before the destructor).
            }

            // Class members declared here can be used by all tests in the test suite
            // for Foo.
        };

        // Tests that the Foo::Bar() method does Abc.
        TEST_F(FooTest, MethodBarDoesAbc) {
            const std::string input_filepath = "this/package/testdata/myinputfile.dat";
            const std::string output_filepath = "this/package/testdata/myoutputfile.dat";
            Foo f;
            //EXPECT_EQ(f.Bar(input_filepath, output_filepath), 0);
        }
    } // !namespace help

}  // !unnamed namespace
