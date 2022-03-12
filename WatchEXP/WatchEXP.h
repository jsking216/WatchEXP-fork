#ifndef __ASHITA_WATCHEXP_H_INCLUDED__
#define __ASHITA_WATCHEXP_H_INCLUDED__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "D:\Ashita\plugins\ADK\Ashita.h"
#include <tuple>
#include <stdint.h>
#include <regex>
#include <time.h>
#include <string>
#include <sstream>
#include <vector>

plugininfo_t* g_PluginInformation = nullptr;

int AbysseaZoneIDs[] = { 15,45,132,215,216,217,218,253,254,255 };
int DynamisZoneIDs[] = { 39,40,41,42,134,135,185,186,187,188 };

class WatchEXP : IPlugin
{
	WatchEXP(const WatchEXP & cpy);
	WatchEXP & operator=(WatchEXP & rhs);

	IAshitaCore*        m_AshitaCore;
	DWORD               m_PluginId;
	IDirect3DDevice8*   m_Direct3DDevice;

public:
	WatchEXP(void);
	virtual ~WatchEXP(void);

public:
    plugininfo_t GetPluginInfo(void) override;

private:
	IFontObject* m_ExpObj;

	char TextObjectBuffer[MAX_PATH];

	time_t m_AbyTimeStart, m_DynTimeStart, m_BaseTimeEXP, m_BaseTimeExemplar,  m_BaseTimeCR, m_resetTime;

	SIZE m_ExpObjSize;
	int font_size, m_reset_min;
	int m_CurrentZoneID, m_LastZoneRecorded;
	char font_name[128];
	char m_Dyna_Crm, m_Dyna_Azr, m_Dyna_Amb, m_Dyna_Alab, m_Dyna_Obs;
	std::string hexColor;
	unsigned int m_vTime;
	unsigned int m_DynaTime;
	unsigned int m_xpKill, m_xpTotal, m_xpTest;
	unsigned int m_crKill, m_crTotal;
	unsigned int m_exemplarKill, m_exemplarTotal, m_exemplarTransitive;
	unsigned int m_prl, m_gld, m_slv, m_ebn, m_azr, m_rby, m_amb;
	unsigned char backTrans;
	bool m_IsInAbyssea, m_IsInDynamis, show_XP_M_always, m_autoreset, m_exemplarMode;
public:
	bool Initialize(IAshitaCore* core, ILogManager* log, uint32_t id) override;
	void Release(void) override;
	bool HandleCommand(const char* pszCommand, int nCommandType) override;
    bool HandleIncomingText(int16_t mode, const char* message, int16_t* modifiedMode, char* modifiedMessage, bool blocked) override;
	bool Direct3DInitialize(IDirect3DDevice8* lpDevice) override;
	void Direct3DRelease(void) override;
	void Direct3DRender(void) override;

	void GetConfig(void);

	float returnProjectedEXPperHour(unsigned int expIntoLevel);
	float returnProjectedExemplarPerHour(unsigned int expIntoLevel);
	float returnProjectedCRUORperHour(unsigned int expIntoLevel);
	void doAbysseaZoneTimings();
	void doDynamisZoneTimings();
	void doDynamisKItemCheck();
	void doEXPCalculations();
	bool isInAbyssea(int zoneID);
	bool isInDynamis(int zoneID);
	void makeComma(int n, char *p, int count);
	int convertFromHex(std::string hex);
	void hextodec(std::string hex, std::vector<unsigned char>& rgb);
	void convertTrans(unsigned char& trans);
	void checkResetTime();
	std::string ScrubChat(const char* chat_line);
};

#endif