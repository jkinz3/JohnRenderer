#pragma once
#include <cassert>
#include <memory>
#include <string>

namespace John
{
	class Image
	{
	public:
		static std::shared_ptr<Image> FromFile(const std::string& filename, int channels = 4);

		int GetWidth() const { return m_width; }
		int GetHeight() const { return m_height; }
		int GetChannels() const { return m_channels; }
		int GetBytesPerPixel() const { return m_channels * (m_hdr ? sizeof(float) : sizeof(unsigned char)); }
		int GetPitch() const { return m_width * GetBytesPerPixel(); }

		bool IsHDR() const { return m_hdr; }

		template<typename T>
		const T* GetPixels() const
		{
			return reinterpret_cast<const T*>(m_pixels.get());
		}
	private:

		Image();
		int m_width;
		int m_height;
		int m_channels;
		bool m_hdr;
		std::unique_ptr<unsigned char> m_pixels;
	};

}