#include "ParamAttribute.h"

#include <vector>
#include <qstringlist.h>

namespace {
	using Dpc::Sybus::ParamAttribute;

	const char ATTRIBUTE_SPLITER = ',';
	const std::vector<std::pair <uint8_t, QString>> FlagsText = {
			{ParamAttribute::WND, "WND"},
			{ParamAttribute::LCL, "LCL"},
			{ParamAttribute::RST, "RST"},
			{ParamAttribute::R, "R"},
			{ParamAttribute::W, "W"},
			{ParamAttribute::PN, "PN"},
			{ParamAttribute::PM, "PM"},
			{ParamAttribute::_2D, "2D"},
			{ParamAttribute::PS, "PS"},
			{ParamAttribute::CFG, "CFG"},
			{ParamAttribute::EXT, "E"},
			{ParamAttribute::NW, "NW"},
			{ParamAttribute::N, "N"}
	};
} // namespace

namespace Dpc::Sybus
{
	ParamAttribute::ParamAttribute(uint16_t value) :
		m_bitset(value)
	{
	}

	ParamAttribute::ParamAttribute(const QString & value) :
		m_bitset(0)
	{
		for (auto &&text : value.split(ATTRIBUTE_SPLITER)) {
			int flag = toFlag(text);
			if (-1 < flag)
				m_bitset[flag] = true;
		}
	}

	bool ParamAttribute::operator[](size_t flag) const
	{
		return m_bitset[flag];
	}

	std::bitset<ParamAttribute::MaxFlagsSize>::reference ParamAttribute::operator[](size_t flag)
	{
		return m_bitset[flag];
	}

	ParamAttribute::operator bool() const
	{
		return toValue();
	}

	uint16_t ParamAttribute::toValue() const
	{
		return static_cast<uint16_t>(m_bitset.to_ulong());
	}

	QString ParamAttribute::toString() const
	{
		QStringList result;
		for (size_t i = 0; i < m_bitset.size(); i++)
			if (m_bitset[i]) {
				auto str = toString((Flags)i);
				if (!str.isEmpty())
					result << str;
			}

		return result.join(ATTRIBUTE_SPLITER);
	}

	QString ParamAttribute::toString(Flags flag)
	{
		for (auto &it : FlagsText)
			if (it.first == flag)
				return it.second;

		return QString();
	}

	int ParamAttribute::toFlag(const QString & text)
	{
		for (auto &it : FlagsText)
			if (it.second == text.trimmed().toUpper())
				return it.first;

		return -1;
	}
} // namespace