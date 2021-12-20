//// JEQ/���ʏ������C�u����/��{�G���[

// ��O�Ƃ��ăG���[���X���[���邱�ƂŁA�܂Ƃ߂ăn���h�����O����B
// �����ł͐F�X�ȃG���[�̊�{�ƂȂ�ȒP�ȃN���X���`����B

#ifndef JEQ_ERROR_HPP
#define JEQ_ERROR_HPP

//// �C���N���[�h

#include <string>

namespace jeq {

//// �^�̒�`

// �G���[��\���B
class error_t {
public:
	error_t() = default;
	virtual ~error_t() = default;
	explicit error_t(const std::string &msg);
	const std::string &getMessage() const;
protected:
	std::string msg; // �G���[���b�Z�[�W�B
};

}

#endif // #ifndef JEQ_ERROR_HPP
