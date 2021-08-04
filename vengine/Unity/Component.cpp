#pragma vengine_package vengine_unity

#include <Component.h>
namespace toolhub::game {
Component::Component(toolhub::db::IJsonDict* dict) {
}
Component::~Component() {
}
}// namespace toolhub::game

#include <Test.hpp>
TestClass::TestClass() {
}
void TestClass::RunData(int32 a, double b) {

}
int32 TestClass::Run1(int32 a, double b) {
	return a + b;
}