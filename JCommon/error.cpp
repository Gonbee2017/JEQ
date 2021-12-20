//// JEQ/共通処理ライブラリ/基本エラー

//// インクルード

#include "error.hpp"

namespace jeq {

//// クラスメンバの定義

// エラーを構築する。
error_t::error_t(
	const std::string &msg // エラーメッセージ。
) : msg(msg) {}

// エラーメッセージを取得する。
const std::string & // 取得したエラーメッセージ。
error_t::getMessage() const {
	return msg;
}

}
