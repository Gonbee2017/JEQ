//// JEQ/���ʏ������C�u����/��{�G���[

//// �C���N���[�h

#include "error.hpp"

namespace jeq {

//// �N���X�����o�̒�`

// �G���[���\�z����B
error::error(
	const std::string &msg // �G���[���b�Z�[�W�B
) : msg(msg) {}

// �G���[���b�Z�[�W���擾����B
const std::string & // �擾�����G���[���b�Z�[�W�B
error::getMessage() const {
	return msg;
}

}
