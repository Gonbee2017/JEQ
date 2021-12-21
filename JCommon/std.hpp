//// JEQ/共通処理ライブラリ/C・C++標準関数インポート

// このプロジェクトで使用しているC・C++標準関数の中で、リソースにアクセス
// するか、またはエラーをチェックする必要のある関数についてインポートする。

#ifndef JEQ_STD_HPP
#define JEQ_STD_HPP

//// インクルード

#include "error.hpp"
#include "import.hpp"

#include <chrono>
#include <condition_variable>
#if _MSVC_LANG >= 201703L
#include <filesystem>
#endif
#include <functional>
#include <ios>
#include <memory>
#include <mutex>
#include <ostream>
#include <string>
#include <thread>

namespace jeq { namespace std_ {

//// 型の定義

// 標準関数のエラーを表す。
// 関数名とエラーナンバーを保持する。
class error_t : public jeq::error_t {
public:
	error_t() = default;
	explicit error_t(const std::string &func_name);
	error_t(const std::string &func_name, int number);
	const std::string &getFunctionName() const;
	int getNumber() const;
protected:
	std::string func_name; // 関数名。
	int number = 0;        // エラーナンバー。
};

//// インポートの宣言

DECL_IMP(chrono_system_clock_now, std::chrono::system_clock::time_point);
DECL_IMP_WRAP(condition_variable_notify_all, void, std::condition_variable *cv);
DECL_IMP_WRAP(condition_variable_wait, void, std::condition_variable *cv, std::unique_lock<std::mutex> *lock, const std::function<bool()> &pred);

#if _MSVC_LANG >= 201703L
DECL_IMP_WRAP(filesystem_equivalent, bool, const std::filesystem::path &p1, const std::filesystem::path &p2);
DECL_IMP(filesystem_exists, bool, const std::filesystem::path &p);
#endif

DECL_IMP_WRAP(getenv, char*, const char *name);
DECL_IMP(geterrno, int);
DECL_IMP_WRAP(make_ofstream, std::shared_ptr<std::ostream>, const std::string &s, std::ios_base::openmode mode);
DECL_IMP_WRAP(make_unique_lock, std::shared_ptr<std::unique_lock<std::mutex>>, std::mutex *mutex);
DECL_IMP_WRAP(make_thread, std::shared_ptr<std::thread>, const std::function<void()> &proc);
DECL_IMP_WRAP(ostream_exceptions_set, void, std::ostream *out, std::ios::iostate except);
DECL_IMP_WRAP(ostream_putLine, void, std::ostream *out, const std::string &line);
DECL_IMP_WRAP(thread_join, void, std::thread *thread);
DECL_IMP_WRAP(unique_lock_unlock, void, std::unique_lock<std::mutex> *lock);

}}

#endif // #ifndef JEQ_STD_HPP
