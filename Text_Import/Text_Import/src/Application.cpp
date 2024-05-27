#include <iostream>
#include <vector>
#include <string>
#include <locale>
#include <codecvt>

#include "hpdf.h"

// 处理错误
void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void* user_data)
{
	std::cerr << "ERROR: " << error_no << ", DETAIL: " << detail_no << std::endl;
}

// 加载所有字符的Unicode范围
std::vector<uint32_t> load_all_unicode_characters()
{
	std::vector<uint32_t> characters;
	for (uint32_t ch = 0x0000; ch <= 0xFFFF; ++ch)
	{
		characters.push_back(ch);
	}
	return characters;
}

// 常量定义
constexpr size_t chars_per_page = 200;
constexpr size_t chars_per_line = 10;
constexpr float page_width = 595.0;
constexpr float page_height = 842.0;
constexpr float margin_left = 50.0;
constexpr float margin_top = 50.0;
constexpr float char_width = 50.0;
constexpr float line_height = 20.0;

int main()
{
	// 初始化libharu库
	HPDF_Doc pdf = HPDF_New(error_handler, nullptr);
	if (!pdf)
	{
		std::cerr << "Error: Cannot create PDFDoc object" << std::endl;
		return 1;
	}

	// 加载字体
	const char* font_path = "C:\\Users\\57394\\AppData\\Local\\Microsoft\\Windows\\Fonts\\Arial Unicode MS.ttf";
	const char* font_name = HPDF_LoadTTFontFromFile(pdf, font_path, HPDF_TRUE);
	if (!font_name)
	{
		std::cerr << "Error: Cannot load font from path: " << font_path << std::endl;
		HPDF_Free(pdf);
		return 1;
	}

	std::cout << "INFO: Font loaded successfully: " << font_name << std::endl;

	// 尝试直接获取字体对象，不设置编码器
	HPDF_Font font = HPDF_GetFont(pdf, font_name, nullptr);
	if (!font)
	{
		std::cerr << "ERROR: Cannot get font object for font name: " << font_name << std::endl;
		HPDF_Free(pdf);
		return 1;
	}

	// 加载所有字符
	std::vector<uint32_t> characters = load_all_unicode_characters();

	// 写入PDF
	size_t num_pages = (characters.size() + chars_per_page - 1) / chars_per_page;

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

	for (size_t page_idx = 0; page_idx < num_pages; ++page_idx)
	{
		HPDF_Page page = HPDF_AddPage(pdf);
		if (!page)
		{
			std::cerr << "ERROR: Cannot create page" << std::endl;
			HPDF_Free(pdf);
			return 1;
		}
		HPDF_Page_SetFontAndSize(page, font, 12);
		HPDF_Page_SetWidth(page, page_width);
		HPDF_Page_SetHeight(page, page_height);

		for (size_t i = 0; i < chars_per_page; ++i)
		{
			size_t char_idx = page_idx * chars_per_page + i;
			if (char_idx >= characters.size())
				break;

			uint32_t ch = characters[char_idx];
			std::wstring ws(1, static_cast<wchar_t>(ch));
			std::string s = converter.to_bytes(ws);

			size_t line = i / chars_per_line;
			size_t col = i % chars_per_line;

			float x = margin_left + col * char_width;
			float y = page_height - margin_top - line * line_height;

			HPDF_Page_BeginText(page);
			HPDF_Page_TextOut(page, x, y, s.c_str());
			HPDF_Page_EndText(page);
		}
	}

	// 保存PDF到文件
	if (HPDF_SaveToFile(pdf, "output.pdf") != HPDF_OK)
	{
		std::cerr << "ERROR: Cannot save PDF to file" << std::endl;
		HPDF_Free(pdf);
		return 1;
	}

	// 释放资源
	HPDF_Free(pdf);

	std::cout << "INFO: PDF created successfully." << std::endl;

	return 0;
}