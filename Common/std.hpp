//// JEQ/���ʏ������C�u����/C�EC++�W���֐��C���|�[�g

// ���̃v���W�F�N�g�Ŏg�p���Ă���C�EC++�W���֐��̒��ŁA���\�[�X�ɃA�N�Z�X
// ���邩�A�܂��̓G���[���`�F�b�N����K�v�̂���֐��ɂ��ăC���|�[�g����B

#ifndef JEQ_STD_HPP
#define JEQ_STD_HPP

//// �C���N���[�h

#include "error.hpp"
#include "import.hpp"

#include <chrono>
#include <filesystem>
#include <ios>
#include <memory>
#include <ostream>
#include <string>

namespace jeq::std_ {

//// �^�̒�`

// �W���֐��̃G���[��\���B
// �֐����ƃG���[�i���o�[��ێ�����B
class error : public jeq::error {
public:
	error() = default;
	explicit error(const std::string &func_name);
	error(const std::string &func_name, int number);
	const std::string &getFunctionName() const;
	int getNumber() const;
protected:
	std::string func_name; // �֐����B
	int number = 0;        // �G���[�i���o�[�B
};

//// �C���|�[�g�̐錾

DECL_IMP(chrono_system_clock_now, std::chrono::system_clock::time_point);
DECL_IMP_WRAP(filesystem_equivalent, bool, const std::filesystem::path &p1, const std::filesystem::path &p2);
DECL_IMP(filesystem_exists, bool, const std::filesystem::path &p);
DECL_IMP_WRAP(getenv, char*, const char *name);
DECL_IMP(geterrno, int);
DECL_IMP_WRAP(make_ofstream, std::shared_ptr<std::ostream>, const std::string &s, std::ios_base::openmode mode);

}

#endif // #ifndef JEQ_STD_HPP
