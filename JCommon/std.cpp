//// JEQ/共通処理ライブラリ/C・C++標準関数インポート

//// インクルード

#include "jcommon.hpp"
#include "std.hpp"

#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <ios>
#include <memory>
#include <thread>
#include <utility>

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

DEF_IMP_DEF(condition_variable_notify_all) = [](
	std::condition_variable *cv
) {
	cv->notify_all();
};

DEF_IMP_WRAP(
	condition_variable_notify_all, 
	void, 
	std::condition_variable *cv
) {
	condition_variable_notify_all_true(cv);
}

DEF_IMP_DEF(condition_variable_wait) = [](
	std::condition_variable *cv, 
	std::unique_lock<std::mutex> *lock, 
	const std::function<bool()> &pred
) {
	cv->wait(*lock, pred);
};

DEF_IMP_WRAP(
	condition_variable_wait, 
	void, 
	std::condition_variable *cv, 
	std::unique_lock<std::mutex> *lock, 
	const std::function<bool()> &pred
) {
	condition_variable_wait_true(cv, lock, pred);
}

DEF_IMP_DEF(filesystem_equivalent) = [](
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

DEF_IMP_DEF(make_ofstream) = [](
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
		ostream_exceptions_set(out.get(), std::ios_base::failbit);
	} catch (const std::ios_base::failure &err) {
		throw error(__FUNCTION__, err.code().value());
	}
	return out;
}

DEF_IMP_DEF(make_unique_lock) = [](
	std::mutex *mutex
) -> std::shared_ptr<std::unique_lock<std::mutex>> {
	return std::make_shared<std::unique_lock<std::mutex>>(*mutex);
};

DEF_IMP_WRAP(
	make_unique_lock, 
	std::shared_ptr<std::unique_lock<std::mutex>>, 
	std::mutex *mutex
) {
	return make_unique_lock_true(mutex);
}

DEF_IMP_DEF(make_thread) = [](
	const std::function<void()> &proc
) -> std::shared_ptr<std::thread> {
	return std::make_shared<std::thread>(proc);
};

DEF_IMP_WRAP(
	make_thread, 
	std::shared_ptr<std::thread>, 
	const std::function<void()> &proc
) {
	return make_thread_true(proc);
}

DEF_IMP_DEF(ostream_exceptions_set) = [](
	std::ostream *out, 
	std::ios::iostate except
) {
	out->exceptions(except);
};

DEF_IMP_WRAP(
	ostream_exceptions_set, 
	void, 
	std::ostream *out, 
	std::ios::iostate except
) {
	ostream_exceptions_set_true(out, except);
}

DEF_IMP_DEF(ostream_putLine) = [](
	std::ostream *out, 
	const std::string &line
) {
	*out << line << '\n';
	out->flush();
};

DEF_IMP_WRAP(
	ostream_putLine,
	void,
	std::ostream *out, 
	const std::string &line
) {
	ostream_putLine_true(out, line);
}

DEF_IMP_DEF(thread_join) = [](
	std::thread *thread
) {
	thread->join();
};

DEF_IMP_WRAP(
	thread_join, 
	void, 
	std::thread *thread
) {
	thread_join_true(thread);
}

DEF_IMP_DEF(unique_lock_unlock) = [](
	std::unique_lock<std::mutex> *lock
) {
	lock->unlock();
};

DEF_IMP_WRAP(
	unique_lock_unlock, 
	void, 
	std::unique_lock<std::mutex> *lock
) {
	unique_lock_unlock_true(lock);
}

}
