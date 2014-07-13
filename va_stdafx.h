///////////////////////////////////////////////////////////////////////////
// googletest macros replacement for visual assist

#define TEST_F(fixture, test_name)\
struct Test_##fixture##_##test_name## : public fixture\
{\
	void Exec();\
};\
void Test_##fixture##_##test_name##::Exec()


#define TEST(test_case_name, test_name)\
struct Test_##test_case_name##_##test_name##\
{\
	void Exec();\
};\
void Test_##test_case_name##_##test_name##::Exec()

#define EXPECT_EQ(a, b) (a) == (b)
