//// JEQ/共通処理ライブラリ/C・C++標準関数インポート

// このプロジェクトで使用しているC・C++標準関数の中で、リソースにアクセス
// するか、またはエラーをチェックする必要のある関数についてインポートする。

#ifndef JEQ_STD_HPP
#define JEQ_STD_HPP

//// インクルード

#include "error.hpp"
#include "import.hpp"

#include <chrono>
#include <filesystem>
#include <ios>
#include <memory>
#include <ostream>
#include <string>

namespace jeq::std_ {

//// 型の定義

// 標準関数のエラーを表す。
// 関数名とエラーナンバーを保持する。
class error : public jeq::error {
public:
	error() = default;
	explicit error(const std::string &func_name);
	error(const std::string &func_name, int number);
	const std::string &getFunctionName() const;
	int getNumber() const;
protected:
	std::string func_name; // 関数名。
	int number = 0;        // エラーナンバー。
};

//// インポートの宣言

DECL_IMP(chrono_system_clock_now, std::chrono::system_clock::time_point);
DECL_IMP_WRAP(filesystem_equivalent, bool, const std::filesystem::path &p1, const std::filesystem::path &p2);
DECL_IMP(filesystem_exists, bool, const std::filesystem::path &p);
DECL_IMP_WRAP(getenv, char*, const char *name);
DECL_IMP(geterrno, int);
DECL_IMP_WRAP(make_ofstream, std::shared_ptr<std::ostream>, const std::string &s, std::ios_base::openmode mode);

}

#endif // #ifndef JEQ_STD_HPP
