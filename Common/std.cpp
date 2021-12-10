//// JEQ/共通処理ライブラリ/C・C++標準関数インポート

//// インクルード

#include "common.hpp"
#include "std.hpp"

#include <cerrno>
#include <fstream>
#include <ios>
#include <memory>

namespace jeq::std_ {

//// クラスメンバの定義

// 標準関数のエラーを構築する。
// エラーナンバーはerrnoから取得する。
error::error(
	const std::string &func_name // 関数名。
) : error(func_name, geterrno()) {}

// 標準関数のエラーを構築する。
// エラーメッセージを作成する。
error::error(
	const std::string &func_name, // 関数名。
	int number                    // エラーナンバー。
) : func_name(func_name), 
	number(number) 
{
	msg = string_printf("%sが失敗しました。(%d)", func_name.c_str(), number);
}

// 関数名を取得する。
const std::string & // 取得した関数名。
error::getFunctionName() const {
	return func_name;
}

// エラーナンバーを取得する。
int // 取得したエラーナンバー。
error::getNumber() const {
	return number;
}

//// インポートの定義

DEF_IMP(chrono_system_clock_now) = std::chrono::system_clock::now;

DEF_IMP_TRUE(filesystem_equivalent) = [](
	const std::filesystem::path &p1, 
	const std::filesystem::path &p2
) -> bool {
	return std::filesystem::equivalent(p1, p2);
};

DEF_IMP_WRAP(
	filesystem_equivalent,
	bool,
	const std::filesystem::path &p1, 
	const std::filesystem::path &p2
) {
	try {
		return filesystem_equivalent_true(p1, p2);
	} catch (const std::system_error &err) {
		throw error(__FUNCTION__, err.code().value());
	}
}

DEF_IMP(filesystem_exists) = [](const std::filesystem::path &p) -> bool {
	return std::filesystem::exists(p);
};

DEF_IMP_WRAP_EASY(
	getenv,
	char*,
	const char *name
) {
	char *ret = getenv_true(name);
	if (!ret) throw error(__FUNCTION__);
	return ret;
}

DEF_IMP(geterrno) = []() -> int {
	return errno;
};

DEF_IMP_TRUE(make_ofstream) = [](
	const std::string &s, 
	std::ios_base::openmode mode
) -> std::shared_ptr<std::ostream> {
	return std::make_shared<std::ofstream>(s, mode);
};

DEF_IMP_WRAP(
	make_ofstream,
	std::shared_ptr<std::ostream>,
	const std::string &s, 
	std::ios_base::openmode mode
) {
	auto out = make_ofstream_true(s, mode);
	try {
		out->exceptions(std::ios_base::failbit);
	} catch (const std::ios_base::failure &err) {
		throw error(__FUNCTION__, err.code().value());
	}
	return out;
}

}
