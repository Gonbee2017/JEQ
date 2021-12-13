//// JEQ/�v���O�C�����[�_�[/�{��

//// �C���N���[�h

#include "dinput8.hpp"
#include "jcommon.hpp"

#include <windows.h>

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

//// �G�N�X�|�[�g�֐��̒�`

// eqgame.exe���炱�̋U����DirectInput8Create���Ăяo���ꂽ��A
// �����ɖ{����DirectInput8Create�𓯈�̈����ŌĂяo�����ƂŁA
// �����������̊֐����{���ł��邩�̂悤�ɐU�镑���B
HRESULT DirectInput8Create(
	HINSTANCE hinst, 
	DWORD dwVersion, 
	REFIID riidltf, 
	LPVOID *ppvOut, 
	LPUNKNOWN punkOuter
) {
	return jeq::context.DirectInput8Create_true(
		hinst, 
		dwVersion, 
		riidltf, 
		ppvOut, 
		punkOuter
	);
}

namespace jeq {

//// �O���[�o���ϐ��̒�`

context_t context; // �v���O�����̕���(��)�B

//// �N���X�����o�̒�`

// �v���O�������J�n����B
// �e�t�@�C���̃p�X��ݒ肵�Adinput8.log���J���B
void context_t::startup(
	HINSTANCE hinstDLL // DLL�C���X�^���X�̃n���h���B
) {
	fake_dll_handle = hinstDLL;
	fake_dll_path = module_getPath(fake_dll_handle);
	dll_name = fake_dll_path.filename().string();
	// �R�}���h���C����������͂���B
	auto command_line = namedValuesToMap(parseArguments(api::GetCommandLine()));
	// �R�}���h���C��������dinput8.log_path���烍�O�t�@�C���̃p�X���擾����B
	// �ȗ��Ȃ�EQ�t�H���_��dinput8.log�̃p�X�ɂȂ�B
	log_path = fake_dll_path;
	log_path.replace_extension("log");
	log_path = map_find(
		command_line, 
		log_path.filename().string() + "_path", 
		log_path.string()
	);
	// �R�}���h���C��������dinput8.ini_path����ݒ�t�@�C���̃p�X���擾����B
	// �ȗ��Ȃ�EQ�t�H���_��dinput8.ini�̃p�X�ɂȂ�B
	ini_path = fake_dll_path;
	ini_path.replace_extension("ini");
	ini_path = map_find(
		command_line, 
		ini_path.filename().string() + "_path", 
		ini_path.string()
	);
	log = std_::make_ofstream(log_path.string(), std::ios_base::app);
}

//// �֐��̒�`

// �v���O�C����DLL�����[�h����B
void loadPlugin(
	const std::string &plugin_name // �v���O�C���̖��O�B
) {
	HMODULE plugin_handle = api::LoadLibrary(plugin_name.c_str());
	context.plugin_handles.emplace_back(plugin_handle);
	context.exits.emplace_back([plugin_handle] {
		api::FreeLibrary(plugin_handle);
	});
}

// ���ׂẴv���O�C����DLL�����[�h����B
// �v���O�C����dinput.ini��Plugin�Z�N�V�����Őݒ肷��B
// �L�[�̖��O��DLL�t�@�C���̖��O�A�l�̓��[�h���邩�ǂ����̃t���O�������B
// ���[�h�t���O��1�Ȃ烍�[�h���邱�ƁA0�Ȃ烍�[�h���Ȃ����Ƃ��Ӗ�����B
/* �T���v���R�[�h
[Plugin]
; ���ׂẴv���O�C����񋓂���B
hoge.dll=1 ; hoge.dll�����[�h����B
fuga.dll=0 ; fuga.dll�����[�h���Ȃ��B
*/
void loadPlugins() {
	// Plugin�Z�N�V�����ɂ��邷�ׂẴL�[���擾����B
	auto plugin_keys = ini_getKeys(context.ini_path, "Plugin");
	// �e�L�[�����񂷂�B
	for (const named_value_t &plugin_key : plugin_keys) {
		// �L�[�̒l�Ŏ�����Ă��郍�[�h�t���O���^�Ȃ�A
		// �L�[�̖��O�Ŏ�����Ă���DLL�t�@�C�������[�h����B
		if (destringize<bool>(plugin_key.value)) loadPlugin(plugin_key.name);
	}
}

// �{����dinput8.dll���������A���[�h���A
// DirectInput8Create�֐��̃A�h���X���擾����B
void loadTrueDLL() {
	// �T�[�`�p�X(���ϐ�PATH)����{����dinput8.dll����������B
	context.true_dll_path = searchTrueDLL();
	if (context.true_dll_path.empty())
		throw error(string_printf(
			"�{����%s��������܂���ł����B", 
			context.dll_name.c_str()
		));
	// ���������p�X����DLL�����[�h����B
	context.true_dll_handle = 
		api::LoadLibrary(context.true_dll_path.string().c_str());
	context.exits.emplace_back([] {
		api::FreeLibrary(context.true_dll_handle);
	});
	// ���[�h����DLL����DirectInput8Create�֐��̃A�h���X���擾����B
	FARPROC proc = api::GetProcAddress(
		context.true_dll_handle, 
		"DirectInput8Create"
	);
	context.DirectInput8Create_true = DirectInput8Create_t(proc);
}

// ����DLL�����[�h���ꂽ�Ƃ��ɌĂяo�����B
// �v���O�������J�n���A�{����dinput8.dll�����[�h���A
// dinput.ini�ɐݒ肳��Ă��邷�ׂẴv���O�C�������[�h����B
// �G���[������������dinput.log�ɏ�������ŁA���s��Ԃ��B
BOOL // �^�Ȃ琬���A�U�Ȃ玸�s�B
onProcessAttach(
	HINSTANCE hinstDLL // DLL�C���X�^���X�̃n���h���B
) {
	try {
		context.startup(hinstDLL);
		loadTrueDLL();
		loadPlugins();
	} catch (const error &err) {
		if (context.log) putLog(*context.log, err.getMessage());
		return FALSE;
	}
	return TRUE;
}

// ����DLL���A�����[�h�����Ƃ��ɌĂяo�����B
// �v���O�C���Ɩ{����dinput8.dll���A�����[�h���Adinput8.log�����B
BOOL // �^�Ȃ琬���A�U�Ȃ玸�s�B
onProcessDetach() {
	reverseClear(context.exits);
	context.log.reset();
	return TRUE;
}

// �T�[�`�p�X(���ϐ�PATH)����{����dinput8.dll����������B
// �{����dinput8.dll�̌�����API��SearchPath�֐����g�����Ƃ͂ł��Ȃ��B
// �Ȃ��Ȃ�SearchPath�͐^����ɃJ�����g�f�B���N�g������������̂ŁA
// �{������ɋU����dinput8.dll(����DLL)�������Ă��܂�����ł���B
std::filesystem::path // ���������p�X�B������Ȃ����������ہB
searchTrueDLL() {
	// PATH�̒l���Z�~�R�����ŕ������A�f�B���N�g���̃R���e�i�Ɋi�[����B
	auto search_dirs = string_split(std_::getenv("PATH"), ';', false);
	// PATH�̊e�f�B���N�g�������񂷂�B
	for (const std::string &search_dir : search_dirs) {
		// ���̃f�B���N�g���ɂ���dinput8.dll�̃p�X���쐬����B
		std::filesystem::path dll_path(search_dir);
		dll_path.append(context.dll_name);
		// dinput8.dll�����݂��A�U���Ɠ����t�@�C���łȂ���΁A
		// ���ꂪ�{����dinput8.dll�ł��邩��A���̃p�X��Ԃ��B
		if (std_::filesystem_exists(dll_path) && 
			!std_::filesystem_equivalent(dll_path, context.fake_dll_path)
		) return dll_path;
	}
	return std::filesystem::path();
}

}
