//// JEQ/共通処理ライブラリ/基本エラー

// 例外としてエラーをスローすることで、まとめてハンドリングする。
// ここでは色々なエラーの基本となる簡単なクラスを定義する。

#ifndef JEQ_ERROR_HPP
#define JEQ_ERROR_HPP

//// インクルード

#include <string>

namespace jeq {

//// 型の定義

// エラーを表す。
class error {
public:
	error() = default;
	virtual ~error() = default;
	explicit error(const std::string &msg);
	const std::string &getMessage() const;
protected:
	std::string msg; // エラーメッセージ。
};

}

#endif // #ifndef JEQ_ERROR_HPP
