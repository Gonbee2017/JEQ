//// JEQ/���ʏ������C�u����/�{��

// ������EXE��DLL�ŋ��ʂ��鏈�������̃��C�u�����ɂ܂Ƃ߂Ĕz�u����B
// ���������̂Ƃ��다�ʂ��Ă��Ȃ��Ă��A�����������Ȕėp�I�ȏ����A
// �Ⴆ�Ε����G���R�[�f�B���O�ϊ��̂悤�ȏ����������ɔz�u����B
// �܂�C++��14��17�̗����A�v���b�g�t�H�[����x86��x64�̗����ɑΉ�����B

#ifndef JEQ_JCOMMON_HPP
#define JEQ_JCOMMON_HPP

//// �C���N���[�h

#include "api.hpp"
#include "dtr.hpp"
#include "error.hpp"
#include "std.hpp"

#include <windows.h>

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <ios>
#include <istream>
#include <ostream>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

//// �}�N���̒�`

// �X�p�C�N���X�̒�`���J�n����B
// �X�p�C�N���X�͒P�̃e�X�g�֐����Œ�`����B�ΏۂƂ���N���X��
// �p�����邱�ƂŁAprotected�����o�ɃA�N�Z�X�ł���悤�ɂ���B
#define BEGIN_DEF_SPY_CLASS(\
	target /* �e�X�g�̑ΏۂƂȂ�N���X�̖��O�B */\
) \
	struct spy_ ## target : public target {\
		static void procedure() {

// �X�p�C�N���X�̒�`�𖼑O��ԕt���ŊJ�n����B
// �X�p�C�N���X�͒P�̃e�X�g�֐����Œ�`����B�ΏۂƂ���N���X��
// �p�����邱�ƂŁAprotected�����o�ɃA�N�Z�X�ł���悤�ɂ���B
#define BEGIN_DEF_SPY_CLASS_N(\
	namespace_, /* �e�X�g�̑ΏۂƂȂ�N���X�̖��O��ԁB */\
	target      /* �e�X�g�̑ΏۂƂȂ�N���X�̖��O�B */\
) \
	struct spy_ ## target : public namespace_::target {\
		static void procedure() {

// �X�p�C�N���X�̒�`���I������B
#define END_DEF_SPY_CLASS(\
	target /* �e�X�g�̑ΏۂƂȂ�N���X�̖��O�B */\
) \
		}\
	};\
	spy_ ## target::procedure();

namespace jeq {

//// �萔���̒�`

// EQ�`���b�g�̃����N�̒萔�B
constexpr char EQCHAT_LINK_EDGE = 0x12; // ���[�̋L���B

//// �萔�̐錾

extern const std::size_t HARDWARE_CONCURRENCY;

//// �^�̒�`

// �啶��/����������ʂ��Ȃ�������̔�r�֐���\���B
struct ignore_case_compare_t {
	inline bool operator ()(const std::string &lhs, const std::string &rhs) const;
};

// ���O�t���̒l��\���B
struct named_value_t {
	std::string name;  // ���O�B
	std::string value; // �l�B
};

// �X�R�[�v����E�o����Ƃ��̏�����\���B
// �C���X�^���X�f�[�^���j�������Ƃ��ɐݒ肳�ꂽ���������s����B
/* �T���v���R�[�h
{ // ���̃u���b�N�̒��Ńf�o�C�X�R���e�L�X�g���g�p����B
	HDC hdc = api::GetDC(NULL); // �f�X�N�g�b�v�̃f�o�C�X�R���e�L�X�g���擾����B
	scope_exit_t hdc_exit([hdc] { // �f�o�C�X�R���e�L�X�g�p�̒E�o�������`����B
		api::ReleaseDC(NULL, hdc); // ���s�����ƃf�o�C�X�R���e�L�X�g���������B
	});
	... // �f�o�C�X�R���e�L�X�g���g�p����B
} // �u���b�N�̃X�R�[�v����E�o����Ƃ��ɐݒ肵�������_�����s�����B
*/
class scope_exit_t {
public:
	// �����̌^�B
	using procedure_t = std::function<void()>;

	scope_exit_t() = default;
	inline explicit scope_exit_t(const procedure_t &proc);
	inline procedure_t &procedure();
protected:
	// �C���X�^���X�f�[�^��\���B
	struct data_t {
		procedure_t proc; // �����B

		inline ~data_t();
	};

	std::shared_ptr<data_t> data = std::make_shared<data_t>(); // �C���X�^���X�f�[�^�B
};

// �P�̃e�X�g��������@�\��\���B
// ���b�N�֐����Ăяo�����Ɗ֐���������̒l�Ȃǂ��L�^����B
// �܂��A�Ԃ���p�����[�^�𐶐�����B
/* �T���v���R�[�h
// cursor_isOverWindow�̒P�̃e�X�g���s���B
test_helper_t help; // �w���p�[�B
// IsWindowVisible�̃��b�N�֐��Ƃ��ă����_��ݒ肷��B
api::IsWindowVisible = [&](HWND hWnd) -> BOOL {
	help << "IsWindowVisible\n"; // �����_���Ăяo���ꂽ���Ƃ��L�^����B
	help << int(hWnd) << '\n'; // ��������hWnd���L�^����B
	return FALSE; // �E�C���h�E����\���Ȃ�cursor_isOverWindow�͋U��Ԃ��͂��B
};
// cursor_isOverWindow�͂܂�api::IsWindowVisible�A�܂�O�q�̃����_���Ăяo���B
Assert::IsFalse(cursor_isOverWindow(help.getSeq<HWND>())); // �U��Ԃ����H
// �Ăяo���������`�F�b�N����B
Assert::AreEqual(std::string("IsWindowVisible"), help.getLine()); // �����_���Ăяo���ꂽ���H
Assert::AreEqual(std::string("1"), help.getLine()); // �n���ꂽhWnd�͐��������H
Assert::AreEqual(std::string(), help.getLine()); // �z��ȏ�ɋL�^����Ă��Ȃ����H
*/
class test_helper_t {
public:
	test_helper_t() = default;
	test_helper_t(const std::string &str);
	std::string getLine();
	template <class X = int>
	X getSeq(std::size_t offset = 0);
	template <class X>
	std::shared_ptr<X> getSeqPtr(std::size_t offset = 0);
	std::string &getSeqStr(std::size_t offset = 0);
	template <typename X>
	test_helper_t &operator <<(const X &x);
	void setSeqBase();
protected:
	// �C���X�^���X�f�[�^��\���B
	struct data_t {
		std::stringstream buff; // �o�b�t�@�B
		int seq = 0;            // �A�ԁB
		int seq_base = 0;       // �A�Ԃ̃x�[�X�B
		std::vector<std::unique_ptr<std::string>> 
			seq_str_buff;       // �A�Ԃ̕�����̃o�b�t�@�B
	};

	std::shared_ptr<data_t> data = std::make_shared<data_t>(); // �C���X�^���X�f�[�^�B
};

// �P�̃e�X�g�Ŏg�p����g�����|�����֐���\���B
// �`�F�b�N�p��std::function���Ăяo���֐����`����B
/* �T���v���R�[�h
// hoge�֐���foo�֐��𐳂����Ăяo���邩���`�F�b�N����B
using test_foo_t = test_trampoline_t<void>; // �g�����|�����֐��𐶐��B
test_helper_t help; // �w���p�[�B
// foo�̃��b�N�֐��Ƃ��ă����_��ݒ肷��B
test_foo_t::target = [&] {
	help << "foo\n"; // �����_���Ăяo���ꂽ���Ƃ��L�^����B
};
// foo�̃g�����|�����֐���ݒ肷��B
foo_target = test_foo_t::trampoline;
// hoge��foo_target�A�܂�test_foo_t::trampoline���Ăяo���B
// test_foo_t::trampoline��test_foo_t::target�A�܂�O�q�̃����_���Ăяo���B
hoge();
// �Ăяo���������`�F�b�N����B
Assert::AreEqual(std::string("foo"), help.getLine()); // �����_���Ăяo���ꂽ���H
*/
template <class Ret, class ...Args> 
struct test_trampoline_t {
	static std::function<Ret(Args...)> target;

	static Ret trampoline(Args ...args);
	static Ret WINAPI trampoline_api(Args ...args);

	Ret trampoline_member(Args ...args);
};

// �X���b�h�v�[����\���B
// �^�X�N�̎��s���˗�����ƁA�v�[�����O����Ă���X���b�h�̒�����
// �ҋ@���̃X���b�h��ڊo�߂����āA���̃X���b�h��Ŕ񓯊��Ɏ��s����B
/* �T���v���R�[�h
// �X���b�h�v�[�����쐬����B
thread_pool_t thread_pool;
// ���[�J�[�X���b�h���N������B
thread_pool.launch();
// ���Z���s���^�X�N���쐬����B
int result;
thread_pool_t::task_t task = [&result] {
	result = 1 + 2;
};
// �^�X�N�̔񓯊����s���˗�����B
auto work = thread_pool.ask(task);
...
// ���[�N(���̗�ł̓^�X�N�P�̂݁j�̎��s����������܂őҋ@����B
work.waitUntilDone();
// ���ʂ��o�͂���B
std::cout << result << std::endl;
*/
class thread_pool_t {
public:
	// �^�X�N�̌^�B
	using task_t = std::function<void()>;

	// ���s���̃^�X�N�̃O���[�v��\���B
	class work_t {
		friend thread_pool_t;
	public:
		work_t() = default;
		bool isDone() const;
		void waitUntilDone();
	protected:
		// �C���X�^���X�f�[�^��\���B
		struct data_t {
			std::mutex mutex;            // �r������̂��߂̃~���[�e�b�N�X�B
			std::size_t tasks_done = 0;  // ���������^�X�N�̐��B
			std::size_t tasks_count = 0; // �^�X�N�̐��B
			std::condition_variable 
				work_cv;                 // ���[�N�ɒʒm���邽�߂̏����ϐ��B
		};

		std::shared_ptr<data_t> data = std::make_shared<data_t>(); // �C���X�^���X�f�[�^�B

		explicit work_t(std::size_t tasks_count);
		void onTaskDone();
	};

	template <class Container>
	work_t ask(const Container &tasks);
	work_t ask(const task_t &task);
	void launch(std::size_t workers_count = HARDWARE_CONCURRENCY);
protected:
	// ���b�N�̌^�B
	using lock_t = std::unique_lock<std::mutex>;

	// �C���X�^���X�f�[�^��\���B
	struct data_t {
		bool leave = false;       // ���E�t���O�B
		                          // �Z�b�g�Ȃ烏�[�J�[���������痣�E����B
		std::mutex mutex;         // �r������̂��߂̃~���[�e�b�N�X�B
		std::queue<task_t> tasks; // �^�X�N�̃L���[�B
		std::vector<std::shared_ptr<std::thread>> 
			workers;              // ���[�J�[�̃x�N�^�B
		std::condition_variable 
			workers_cv;           // ���[�J�[�ɒʒm���邽�߂̏����ϐ��B

		~data_t();
	};

	std::shared_ptr<data_t> data = std::make_shared<data_t>(); // �C���X�^���X�f�[�^�B

	void worker_procedure();
};

//// �e���v���[�g�֐��ƃC�����C���֐��̐錾

inline bool char_isJLead(char chr);
template <class X>
void clampByMin(X &value, const X &min);
template <class X>
X destringize(const std::string &str, X def);
template <class X, class Manip>
X destringize(const std::string &str, X def, Manip &&manip);
inline BYTE getCtrlAlphabetKey(char alphabet);
template <class PostX, class PreX>
PostX indirect_cast(PreX x);

#if _MSVC_LANG >= 201703L
template <template <class> class Container = std::vector>
#else
template <template <class> class Container>
#endif
Container<named_value_t> ini_getKeys(const std::string &ini_path, const std::string &section_name);

#if _MSVC_LANG >= 201703L
template <template <class> class Container = std::vector>
#else
template <template <class> class Container>
#endif
Container<named_value_t> ini_parseSection(const std::string &section);

template <class Map>
typename Map::mapped_type map_find(const Map &map, const typename Map::key_type &key, const typename Map::mapped_type &def_mapped = typename Map::mapped_type());
template <class NamedValues>
std::map<std::string,std::string,ignore_case_compare_t> namedValuesToMap(const NamedValues &named_values);

#if _MSVC_LANG >= 201703L
template <template <class> class Container = std::vector>
#else
template <template <class> class Container>
#endif
Container<named_value_t> parseArguments(const std::string &args);

template <class Container>
void reverseClear(Container &container);
template <class Number>
bool section_areOverlapped(const Number &begin1, const Number &end1, const Number &begin2, const Number &end2);
template <class X>
std::string stringize(const X &x);
template <class X, class Manip>
std::string stringize(const X &x, Manip &&manip);
template <class X, class Lead, class ...Trails>
std::string stringize(const X &x, Lead &&lead, Trails &&...trails);

#if _MSVC_LANG >= 201703L
template <template <class> class Container = std::vector>
#else
template <template <class> class Container>
#endif
Container<std::string> string_split(const std::string &str,	char delim,	bool allow_empty = true);

//// ����J�ȃe���v���[�g�֐��ƃC�����C���֐��̐錾

template <class X>
X _destringizeFrom(std::istream &in, X def);
template <class X, class Manip>
X _destringizeFrom(std::istream &in, X def, Manip &&manip);
template <class X>
void _stringizeTo(std::ostream &out, const X &x);
template <class X, class Manip>
void _stringizeTo(std::ostream &out, const X &x, Manip &&manip);
template <class X, class Lead, class ...Trails>
void _stringizeTo(std::ostream &out, const X &x, Lead &&lead, Trails &&...trails);

//// �֐��̒�`

POINT cursor_getPos();
bool cursor_isOverWindow(HWND hwnd);
void ini_deleteKey(const std::string &ini_path, const std::string &section_name, const std::string &key_name);
void ini_deleteSection(const std::string &ini_path, const std::string &section_name);
std::string ini_getKeyValue(const std::string &ini_path, const std::string &section_name, const std::string &key_name, const std::string &def_value = std::string());
std::string ini_loadSection(const std::string &ini_path, const std::string &section_name);
void ini_setKeyValue(const std::string &ini_path, const std::string &section_name, const std::string &key_name, const std::string key_value);
std::string module_getPath(HMODULE hmod);
POINT point_clientToScreen(POINT pos, HWND hwnd);
POINT point_screenToClient(POINT pos, HWND hwnd);
void putLog(std::ostream *log, const std::string &mes);
bool rect_areOverlapped(const RECT &rect1, const RECT &rect2);
std::string string_printf(const char *fmt, ...);
std::string string_sjisToUtf8(const std::string &src);
std::wstring string_sjisToUtf16(const std::string &src);
std::string string_toLower(const std::string &src);
RECT window_getClientRect(HWND hwnd);
RECT window_getRect(HWND hwnd);
std::string window_getText(HWND hwnd);
bool window_isTopMost(HWND hwnd);
std::string wstring_utf16ToUtf8(const std::wstring &src);

//// �N���X�̃e���v���[�g�����o�ƃC�����C�������o�̒�`

// 2�̕������啶��/�������̋�ʂȂ��Ŕ�r����B
bool // �^�Ȃ�lhs<rhs�A�U�Ȃ�lhs>=rhs�B
ignore_case_compare_t::operator ()(
	const std::string &lhs, // �����̕�����B
	const std::string &rhs  // �E���̕�����B
) const {
	return _stricmp(lhs.c_str(), rhs.c_str()) < 0;
}

// �E�o�������\�z����B
scope_exit_t::scope_exit_t(
	const procedure_t &proc // ���s����鏈���B
) {
	data->proc = proc;
}

// �E�o�����̃C���X�^���X�f�[�^���j�������Ƃ��Ɏ��s����鏈�����Q�Ƃ���B
scope_exit_t::procedure_t & // �Q�Ƃ������s����鏈���B
scope_exit_t::procedure() {
	return data->proc;
}

// �E�o�����̃C���X�^���X�f�[�^��j������B
// �������ݒ肳��Ă���΂����Ŏ��s����B
scope_exit_t::data_t::~data_t() {
	if (proc) proc();
}

// �e�X�g�w���p�[���玟�̘A�Ԃ��擾����B
// �A�Ԃ�1����n�܂�̂Œ��ӂ��邱�ƁB
template <
	class X // �����̌^�B
> X // �擾�����A�Ԃ��L���X�g���������B
test_helper_t::getSeq(
	std::size_t offset // �x�[�X����̃I�t�Z�b�g�B
	                   // �[���Ȃ�ŐV�̘A�ԁB�ȗ��Ȃ�[���B
) {
	++data->seq;
	int seq;
	if (offset) seq = data->seq_base + int(offset);
	else seq = data->seq;
	return X(seq);
}

// �e�X�g�w���p�[���玟�̘A�Ԃ������̃A�h���X�Ƃ���|�C���^���擾����B
// �|�C���^�̃f���[�^�͉������Ȃ��B
template <
	class X // �����̌^�B
> std::shared_ptr<X> // �擾�����A�Ԃ������̃A�h���X�Ƃ���|�C���^�B
test_helper_t::getSeqPtr(
	std::size_t offset // �x�[�X����̃I�t�Z�b�g�B
                       // �[���Ȃ�ŐV�̘A�ԁB�ȗ��Ȃ�[���B
) {
	return std::shared_ptr<X>(getSeq<X*>(offset), [](X*) {});
}

// �e�X�g�w���p�[�ɉ������L�^����B
template <
	typename X // �L�^���鉽���̌^�B
> test_helper_t & // �Ăяo�������B
test_helper_t::operator <<(
	const X &x // �L�^���鉽���B
) {
	data->buff << x;
	return *this;
}

// �P�̃e�X�g�Ńg�����|�����֐�����Ăяo�����^�[�Q�b�g�֐��B
template <
	class Ret,    // �߂�l�̌^�B
	class ...Args // �����̌^�B
> std::function<Ret(Args...)> test_trampoline_t<Ret,Args...>::target;

// �P�̃e�X�g�Ń^�[�Q�b�g�֐����Ăяo���g�����|�����֐��B
template <
	class Ret,    // �߂�l�̌^�B
	class ...Args // �����̌^�B
> Ret // �߂�l�B
test_trampoline_t<Ret,Args...>::trampoline(
	Args ...args // �����B
) {
	return target(args...);
}

// �P�̃e�X�g�Ń^�[�Q�b�g�֐����Ăяo��API�����g�����|�����֐��B
template <
	class Ret,    // �߂�l�̌^�B
	class ...Args // �����̌^�B
> Ret // �߂�l�B
WINAPI test_trampoline_t<Ret,Args...>::trampoline_api(
	Args ...args // �����B
) {
	return target(args...);
}

// �P�̃e�X�g�Ń^�[�Q�b�g�֐����Ăяo�������o�����g�����|�����֐��B
template <
	class Ret,    // �߂�l�̌^�B
	class ...Args // �����̌^�B
> Ret // �߂�l�B
test_trampoline_t<Ret,Args...>::trampoline_member(
	Args ...args // �����B
) {
	return target(args...);
}

// �X���b�h�v�[���Ƀ^�X�N�R���e�i�̔񓯊����s���˗�����B
template <
	class Container // �^�X�N�R���e�i�̌^�B
> thread_pool_t::work_t // �쐬�������[�N�B
thread_pool_t::ask(
	const Container &tasks // �񓯊����s����^�X�N�̃R���e�i�B
) {
	// �^�X�N���������郏�[�N���쐬����B
	work_t work(tasks.size());
	// �^�X�N���L���[�Ƀv�b�V������B
	auto lock = std_::make_unique_lock(&data->mutex);
	for (const task_t &task : tasks)
		data->tasks.push([work, task]() mutable {
			// ���̃����_�̓��[�J�[�X���b�h�Ŕ񓯊��Ɏ��s�����B
			// �^�X�N�{�̂����s���A����������n���h�����Ăяo���B
			task();
			work.onTaskDone();
		});
	std_::unique_lock_unlock(lock.get());
	// ���[�J�[��ڊo�߂����āA�L���[�Ƀv�b�V�������^�X�N�����s������B
	std_::condition_variable_notify_all(&data->workers_cv);
	return work;
}

//// �e���v���[�g�֐��ƃC�����C���֐��̒�`

// �������V�t�gJIS��1�o�C�g�ڂ��ǂ����𔻒肷��B
bool // �^�Ȃ�V�t�gJIS��1�o�C�g�ځA�U�Ȃ炻��ȊO�B
char_isJLead(
	char chr // �����B
) {
	BYTE byte = BYTE(chr);
	return (byte >= 0x81 && byte <= 0x9f) || (byte >= 0xe0 && byte <= 0xfc);
}

// �l���ŏ��l�ȏ�ɂ���B
template <
	class X // �l�̌^�B
>
void clampByMin(
	X &value,    // �l�Bmin��菬�����Ƃ���min��������B
	const X &min // �ŏ��l�B
) {
	if (value < min) value = min;
}

// ������������ɕϊ�����B
/* �T���v���R�[�h
// ������𐮐��ɕϊ�����B
int num = destringize<int>("123", 0);
*/
template <
	class X // �����̌^�B
> X // �ϊ����������B���s�����Ƃ��͊���l�B
destringize(
	const std::string &str, // �ϊ����镶����B
	X def                   // ����l�B
) {
	if (str.empty()) return X();
	std::istringstream in(str);
	return _destringizeFrom<X>(in, def);
}

// ������������ɕϊ�����B
// �܂��ϊ��̕��@���}�j�s�����[�^�Ŏw�肷��B
/* �T���v���R�[�h
// 16�i���̕�����𐮐��ɕϊ�����B
int num = destringize<int>("abc", 0, std::hex);
*/
template <
	class X,    // �����̌^�B
	class Manip // �}�j�s�����[�^�̌^�B
> X // �ϊ����������B���s�����Ƃ��͊���l�B
destringize(
	const std::string &str, // �ϊ����镶����B
	X def,                  // ����l�B
	Manip &&manip           // �}�j�s�����[�^�B
) {
	if (str.empty()) return X();
	std::istringstream in(str);
	return _destringizeFrom<X>(in, def, manip);
}

// Ctrl+�A���t�@�x�b�g�̉��z�L�[�R�[�h���擾����B
BYTE // �擾�������z�L�[�R�[�h�B
getCtrlAlphabetKey(
	char alphabet // �擾����A���t�@�x�b�g(�啶��)�B
) {
	return 0x01 + (alphabet - 'A');
}

// �������ԐړI�ɃL���X�g����B
// �����Ă��̒l�͒��ɃL���X�g�ł��邪�A�Ⴆ�΃����o�֐��̃|�C���^��
// ���ɃL���X�g�ł��Ȃ��B�����Œl����������ꎞ�ϐ��ɑ�����āA
// ���̃|�C���^���L���X�g���邱�ƂŁA�ԐړI�ɒl���L���X�g����B
/* �T���v���R�[�h
class Hoge {
public: void foo() { ... }
};
...
// Hoge�N���X�̃����o�֐�foo�̃A�h���X���擾�������B
DWORD ng1 = DWORD(&Hoge::foo); // ���ɃL���X�g���悤�Ƃ���ƃG���[�ɂȂ�B
DWORD ng2 = reinterpret_cast<DWORD>(&Hoge::foo); // ������G���[�ɂȂ�B
DWORD ok = indirect_cast<DWORD>(&Hoge::foo); // �ԐړI�ɃL���X�g�����OK�B
*/
template <
	class PostX, // �L���X�g��̉����̌^�B
	class PreX   // �L���X�g�O�̉����̌^�B
> PostX // �L���X�g���������B
indirect_cast(
	PreX x // �L���X�g���鉽���B
) {
	return *(PostX*)(&x);
}

// �ݒ�t�@�C���̃Z�N�V�����ɂ��邷�ׂẴL�[���擾����B
template <
	template <class> class Container // �L�[�̃R���e�i�̌^�B
> Container<named_value_t> // �擾�����L�[�̃R���e�i�B
ini_getKeys(
	const std::string &ini_path,    // �ݒ�t�@�C���̃p�X�B
	const std::string &section_name // �Z�N�V�����̖��O�B
) {
	return ini_parseSection<Container>(ini_loadSection(ini_path, section_name));
}

// �ݒ�t�@�C���̃Z�N�V��������͂���B
// GetPrivateProfileSection�œǂݍ��񂾃Z�N�V������
// '\0'��؂�ŃL�[�����сA�I�[��'\0'��2�����B
// �܂��L�[��'='�Ŗ��O�ƒl����؂��Ă���B
// �����ł̓Z�N�V��������͂��ăL�[�̃R���e�i���쐬����B
template <
	template <class> class Container // �L�[�̃R���e�i�̌^�B
> Container<named_value_t> // �쐬�����L�[�̃R���e�i�B
ini_parseSection(
	const std::string &section // �Z�N�V�����B
) {
	Container<named_value_t> keys;
	named_value_t key = {};
	std::string *cur = &key.name;
	char pre_chr = '\0';
	auto chr_iter = section.begin();
	for (;;) {
		char chr = *chr_iter++;
		if (chr) {
			if (char_isJLead(chr)) {
				cur->append(1, chr);
				chr = *chr_iter++;
				if (chr) cur->append(1, chr);
			} else if (chr == '=') cur = &key.value;
			else cur->append(1, chr);
		} else if (pre_chr) {
			keys.emplace_back(key);
			key = {};
			cur = &key.name;
		} else break;
		pre_chr = chr;
	}
	if (!key.name.empty()) keys.emplace_back(key);
	return keys;
}

// �}�b�v����T���B
template <
	class Map // �}�b�v�̌^�B
> typename Map::mapped_type // ���������}�b�v�l�B������Ȃ����������l�B
map_find(
	const Map &map,                             // �}�b�v�B
	const typename Map::key_type &key,          // �T���L�[�B
	const typename Map::mapped_type &def_mapped // ����l�B
) {
	auto iter = map.find(key);
	if (iter == map.end()) return def_mapped;
	return iter->second;
}

// ���O�t���̒l�̃R���e�i���}�b�v�ɕϊ�����B
// ���O���d�����Ă���ꍇ�A�ŏ��̗v�f�̒l�ɂȂ�B
// ���O�͑啶��/����������ʂ��Ȃ��B
template <
	class NamedValues // ���O�t���̒l�̃R���e�i�̌^�B
> std::map<std::string,std::string,ignore_case_compare_t> // �ϊ������}�b�v�B
namedValuesToMap(
	const NamedValues &named_values // �ϊ����閼�O�t���̒l�̃R���e�i�B
) {
	std::map<std::string,std::string,ignore_case_compare_t> map;
	for (const named_value_t &named_value : named_values)
		map.emplace(named_value.name, named_value.value);
	return map;
}

// �����̕��������͂��A���O�t���̒l�̃R���e�i���쐬����B
// �����͉��ł��L�q�\�ŁA�����ƈ����̊Ԃ̓X�y�[�X�ŋ�؂�B
// ����1 ����2 ... ����n
// ���ꂼ������͎��̂悤�Ȍ`���ɂȂ�B
// "NAME[=VALUE]"
// ������NAME�͈����̖��O�AVALUE�͈����̒l�������B
// ���O�ƒl�ɃX�y�[�X(' ')�ƃC�R�[��('=')���܂߂����Ƃ��́A
// �������܂ޕ���������p��('"')�ň͂ށB
// ���p���ň͂񂾕�����̒��Ɉ��p�����܂߂����Ƃ��́A
// ���p����A���ŏ����΂悢�B
template <
	template <class> class Container // ���O�t���̒l�̃R���e�i�̌^�B
> Container<named_value_t> // �쐬�������O�t���̒l�̃R���e�i�B
parseArguments(
	const std::string &args
) {
	Container<named_value_t> named_values;
	std::string name;
	std::string value;
	std::string *token = &name;
	auto flush = [&] {
		if (!name.empty()) {
			named_values.emplace_back(named_value_t{name, value});
			name.clear();
			value.clear();
		}
	};
	for (auto iter = args.begin(); iter != args.end();)	{
		char chr = *iter++;
		if (chr == '"') {
			char pre_chr = '\0';
			while (iter != args.end()) {
				chr = *iter++;
				if (chr == '"') {
					if (pre_chr == '"') {
						token->append(1, chr);
						pre_chr = '\0';
					} else pre_chr = chr;
				} else if (pre_chr == '"') break;
				else {
					token->append(1, chr);
					if (iter != args.end() && char_isJLead(chr)) {
						token->append(1, *iter++);
						pre_chr = '\0';
					} else pre_chr = chr;
				}
			}
			if (chr == '"') break;
		}
		if (chr == ' ') {
			flush();
			token = &name;
		} else if (token == &name && chr == '=')
			token = &value;
		else {
			token->append(1, chr);
			if (iter != args.end() && char_isJLead(chr))
				token->append(1, *iter++);
		}
	}
	flush();
	return named_values;
}

// �R���e�i���t���ɃN���A����B
// ���\�[�X�̊l���ɂ́A����ȑO�Ɋl�������ʂ̃��\�[�X�𗘗p���邱�Ƃ������B
// �Ⴆ�΃p�^�[���u���V�̍쐬�ɂ́A���[�h�ς݂̃r�b�g�}�b�v���K�v�ɂȂ�B
// �������g�p���I���ĕs�v�ɂȂ�����A�r�b�g�}�b�v���u���V�Ƃ����l������
// ���ł͂Ȃ��A�t�̃u���V���r�b�g�}�b�v�Ƃ������ō폜���Ȃ���΂Ȃ�Ȃ��B
// ���̂��߃��\�[�X���l���������ŃR���e�i�ɍ폜������ǉ����Ă����āA
// �s�v�ɂȂ�����ǉ������Ƃ��Ƃ͋t�̏��ō폜���������s����΂��܂������B
/* �T���v���R�[�h
std::vector<scope_exit_t> exits; // �폜�����̃R���e�i�B
// �r�b�g�}�b�v�����[�h����B
HBITMAP hbmp = api::LoadBitmap(hinst, MAKEINTRESOURCE(IDB_X));
// �r�b�g�}�b�v�p�̍폜������ǉ�����B
exits.emplace_back([hbmp] {
	api::DeleteObject(hbmp); // �@���s�����ƃr�b�g�}�b�v���폜����B
});
// �r�b�g�}�b�v����p�^�[���u���V���쐬����B
HBRUSH hbrush = api::CreatePatternBrush(hbmp);
// �p�^�[���u���V�p�̍폜������ǉ�����B
exits.emplace_back([hbrush] {
	api::DeleteObject(hbrush); // �A���s�����ƃp�^�[���u���V���폜����B
});
... // �p�^�[���u���V���g�p����B
// �R���e�i���t���ɃN���A����΁A�폜�����͇A���@�̏��Ŏ��s�����B
reverseClear(exits);
*/
template <
	class Container // �R���e�i�̌^�B
> void reverseClear(
	Container &container // �R���e�i�B
) {
	while (!container.empty()) container.pop_back();
}

// �Q�̋�Ԃ��d�����Ă��邩�ǂ����𔻒肷��B
// ��Ԃ�[begin, end)�ł���Aend�͊܂܂Ȃ��B
template <
	class Number // ���l�̌^�B
> bool // �^�Ȃ�d�����Ă���A�U�Ȃ�d�����Ă��Ȃ��B
section_areOverlapped(
	const Number &begin1, // �P�ڂ̋�Ԃ̎n�[�l�B
	const Number &end1,   // �P�ڂ̋�Ԃ̏I�[�l�B
	const Number &begin2, // �Q�ڂ̋�Ԃ̎n�[�l�B
	const Number &end2    // �Q�ڂ̋�Ԃ̏I�[�l�B
) {
	return (begin1 >= begin2 && begin1 < end2) ||
		(end1 > begin2 && end1 <= end2) ||
		(begin1 < begin2 && end1 > end2);
}

// �����𕶎���ɕϊ�����B
/* �T���v���R�[�h
// ������𐮐��ɕϊ�����B
std::string str = stringize(123);
*/
template <
	class X // �����̌^�B
> std::string // �ϊ�����������B
stringize(
	const X &x // �ϊ����鉽���B
) {
	std::ostringstream out;
	_stringizeTo<X>(out, x);
	return out.str();
}

// �����𕶎���ɕϊ�����B
// �܂��ϊ��̕��@���}�j�s�����[�^�Ŏw�肷��B
/* �T���v���R�[�h
// ������16�i���̕�����ɕϊ�����B
std::string str = stringize(0xabc, std::hex);
*/
template <
	class X,    // �����̌^�B
	class Manip // �}�j�s�����[�^�̌^�B
> std::string // �ϊ�����������B
stringize(
	const X &x,   // �ϊ����鉽���B
	Manip &&manip // �}�j�s�����[�^�B
) {
	std::ostringstream out;
	_stringizeTo<X>(out, x, manip);
	return out.str();
}

// �����𕶎���ɕϊ�����B
// �܂��ϊ��̕��@�𕡐��̃}�j�s�����[�^�Ŏw�肷��B
/* �T���v���R�[�h
// ������8����16�i���̕�����ɕϊ�����B
std::string str = stringize(0xabc, std::hex, std::setw(8), std::setfill('0'));
*/
template <
	class X,        // �����̌^�B
	class Lead,     // 1�ڂ̃}�j�s�����[�^�̌^�B
	class ...Trails // 2�ڈȍ~�̃}�j�s�����[�^�̌^�B
> std::string // �ϊ�����������B
stringize(
	const X &x,        // �ϊ����鉽���B
	Lead &&lead,       // 1�ڂ̃}�j�s�����[�^�B
	Trails &&...trails // 2�ڈȍ~�̃}�j�s�����[�^�B
) {
	std::ostringstream out;
	_stringizeTo<X>(out, x, lead, trails...);
	return out.str();
}

// ���������؂蕶���ŕ�������B
template <
	template <class> class Container // ������̃R���e�i�̌^�B
> Container<std::string> // ��������������̃R���e�i�B
string_split(
	const std::string &str, // �������镶����B
	char delim,             // ��؂蕶���B
	bool allow_empty        // �󕶎�����R���e�i�ɒǉ����邩�ǂ����B
	                        // �^�Ȃ�ǉ����A�U�Ȃ�ǉ����Ȃ��B�ȗ�����Ɛ^�B
) {
	Container<std::string> toks;
	std::string tok;
	auto flush = [allow_empty, &toks, &tok] {
		if (allow_empty || !tok.empty()) {
			toks.emplace_back(tok);
			tok.clear();
		}
	};
	for (auto iter = str.begin(); iter != str.end();) {
		char chr = *iter++;
		if (chr == delim) flush();
		else {
			tok.push_back(chr);
			if (char_isJLead(chr) && iter != str.end()) 
				tok.push_back(*iter++);
		}
	}
	flush();
	return toks;
}

//// ����J�ȃe���v���[�g�֐��ƃC�����C���֐��̒�`

// �X�g���[�����牽������͂���B
template <
	class X // �����̌^�B
> X // ���͂��������B���s�����Ƃ��͊���l�B
_destringizeFrom(
	std::istream &in, // ���̓X�g���[���B
	X def             // ����l�B
) {
	X x;
	in.exceptions(std::ios_base::failbit);
	try {
		in >> x;
	} catch (const std::ios_base::failure&) {
		x = def;
	}
	return x;
}

// �X�g���[�����牽������͂���B
// �܂����͂̕��@���}�j�s�����[�^�Ŏw�肷��B
template <
	class X,    // �����̌^�B
	class Manip // �}�j�s�����[�^�̌^�B
> X // ���͂��������B���s�����Ƃ��͊���l�B
_destringizeFrom(
	std::istream &in, // ���̓X�g���[���B
	X def,            // ����l�B
	Manip &&manip     // �}�j�s�����[�^�B
) {
	in >> manip;
	return _destringizeFrom<X>(in, def);
}

// �������X�g���[���ɏo�͂���B
template <
	class X // �����̌^�B
> void _stringizeTo(
	std::ostream &out, // �o�̓X�g���[���B
	const X &x         // �o�͂��鉽���B
) {
	out << x;
}

// �������X�g���[���ɏo�͂���B
// �܂��o�͂̕��@���}�j�s�����[�^�Ŏw�肷��B
template <
	class X,    // �����̌^�B
	class Manip // �}�j�s�����[�^�̌^�B
> void _stringizeTo(
	std::ostream &out, // �o�̓X�g���[���B
	const X &x,        // �o�͂��鉽���B
	Manip &&manip      // �}�j�s�����[�^�B
) {
	out << manip;
	_stringizeTo(out, x);
}

// �������X�g���[���ɏo�͂���B
// �܂��o�͂̕��@�𕡐��̃}�j�s�����[�^�Ŏw�肷��B
template <
	class X,        // �����̌^�B
	class Lead,     // 1�ڂ̃}�j�s�����[�^�̌^�B
	class ...Trails // 2�ڈȍ~�̃}�j�s�����[�^�̌^�B
> void _stringizeTo(
	std::ostream &out, // �o�̓X�g���[���B
	const X &x,        // �o�͂��鉽���B
	Lead &&lead,       // 1�ڂ̃}�j�s�����[�^�B
	Trails &&...trails // 2�ڈȍ~�̃}�j�s�����[�^�B
) {
	out << lead;
	_stringizeTo(out, x, trails...);
}

}

#endif // #ifndef JEQ_JCOMMON_HPP
