#include "WatchEXP.h"
#include "CommandParse.h"

WatchEXP::WatchEXP(void)
    : m_AshitaCore(nullptr)
    , m_PluginId(0)
    , m_Direct3DDevice(nullptr)
    , m_ExpObj(nullptr)
    , m_AbyTimeStart(0)
    , m_DynTimeStart(0)
    , m_BaseTimeEXP(0)
    , m_BaseTimeExemplar(0)
    , m_BaseTimeCR(0)
    , m_resetTime(NULL)
    , font_size(0)
    , m_reset_min(0)
    , m_CurrentZoneID(0)
    , m_LastZoneRecorded(0)
    , m_Dyna_Crm('X')
    , m_Dyna_Azr('X')
    , m_Dyna_Amb('X')
    , m_Dyna_Alab('X')
    , m_Dyna_Obs('X')
    , m_vTime(0)
    , m_DynaTime(0)
    , m_xpKill(0)
    , m_xpTotal(0)
    , m_xpTest(0)
    , m_exemplarKill(0)
    , m_exemplarTotal(0)
    , m_exemplarTransitive(0)
    , m_crKill(0)
    , m_crTotal(0)
    , m_prl(0)
    , m_gld(0)
    , m_slv(0)
    , m_ebn(0)
    , m_azr(0)
    , m_rby(0)
    , m_amb(0)
    , backTrans(0)
    , m_IsInAbyssea(false)
    , m_IsInDynamis(false)
    , show_XP_M_always(false)
    , m_autoreset(false)
    , m_exemplarMode(false)
{ }
WatchEXP::~WatchEXP(void)
{ }

plugininfo_t WatchEXP::GetPluginInfo(void)
{
    return *g_PluginInformation;
}

bool WatchEXP::Initialize(IAshitaCore* core, ILogManager* log, uint32_t id)
{
    // Store the variables we are passed..
    this->m_AshitaCore = core;
    this->m_PluginId = id;

    this->GetConfig();

    return true;
}

void WatchEXP::Release(void)
{
}

bool WatchEXP::HandleCommand(const char* pszCommand, int nCommandType)
{
    if (pszCommand == nullptr)
        return false;

    if (_strnicmp(pszCommand, "/wexp", 4) == 0)
    {
        std::vector<std::string> args;
        Ashita::Commands::GetCommandArgs(pszCommand, &args);

        if (args[1] == "reset")
        {
            m_crTotal = 0, m_xpTotal = 0, m_xpKill = 0, m_crKill = 0, m_exemplarTotal = 0, m_exemplarKill = 0;
            m_BaseTimeEXP = time(nullptr), m_BaseTimeCR = time(nullptr), m_BaseTimeExemplar = time(nullptr);

            m_AshitaCore->GetChatManager()->Write("WatchEXP was reset!");
            return true;
        }
        else if (args[1] == "config")
        {
            this->GetConfig();
            m_ExpObj->SetFontFamily(font_name);
            m_ExpObj->SetFontHeight(font_size);

            return true;
        }
        else if (args[1] == "exemplar")
        {
            m_exemplarMode = true;

            return true;
        }
    }
    return false;
}

//bool WatchEXP::HandleNewChatLine(short sMode, char* pszChatLine)
bool WatchEXP::HandleIncomingText(int16_t sMode, const char* pszChatLine, int16_t* modifiedMode, char* modifiedMessage, bool blocked)
{
    std::string data;
    std::vector< std::string > temp;

    data = ScrubChat(pszChatLine);

    //Holds matches
    std::smatch m;

#pragma region Search Pattern
//Regex Searches
    std::regex xp_inc("gains\\s\\d+\\s(experience|limit)\\spoints");
    std::regex xp_chain("EXP\\schain\\s\\d+\\s\\w+\\sgains\\s\\d+\\sexperience\\spoints");
    std::regex exemplar_inc("gains\\s\\d+\\sexemplar\\spoints");
    std::regex exemplar_chain("Master\\schain\\s\\d+\\s\\w+\\sgains\\s\\d+\\sexemplar\\spoints");
    std::regex cru_inc("obtained\\s\\d+\\scruor", std::regex_constants::icase);
    std::regex vis_wear("Your\\svisitant\\sstatus\\swill\\swear\\soff\\sin\\s\\d+\\sminutes");
    std::regex vis_inc("Your\\svisitant\\sstatus\\shas\\sbeen\\sextended\\sby\\s\\d+\\sminutes");
    std::regex vis_exit("Exiting\\sin\\s\\d+\\sminutes");
    std::regex vis_pegs("Visitant\\sLight\\sIntensity\\sPearlescent\\s\\d+\\sEbon\\s\\d+\\sGolden\\s\\d+\\sSilvery\\s\\d+");
    std::regex vis_azram("Azure\\s\\d+\\sRuby\\s\\d+\\sAmber\\s\\d+");
    std::regex light_pearl("body\\semits\\sa\\sfaint\\spearlescent\\slight");
    std::regex light_azure("body\\semits\\sa\\sfeeble\\sazure\\slight");
    std::regex light_ruby("body\\semits\\sa\\sfeeble\\sruby\\slight");
    std::regex light_amber("body\\semits\\sa\\sfeeble\\samber\\slight");
    std::regex light_gold("body\\semits\\sa\\sfaint\\sgolden\\slight");
    std::regex light_silver("body\\semits\\sa\\sfaint\\ssilvery\\slight");
    std::regex light_ebon("body\\semits\\sa\\sfaint\\sebon\\slight");
    std::regex dyn_start("sands");
    std::regex dyn_start2("hourglass");
    std::regex dyn_extend("Dynamis");
    std::regex dyn_extend2("extended");
    std::regex dyn_expelled("expelled");

    //Used to get all the numbers
    std::regex get_number("\\d+");

    if (sMode == 121)
    {
        if (std::regex_search(data, xp_chain))
        {
            std::sregex_token_iterator start(data.begin(), data.end(), get_number, 0), end;

            while (start != end)
            {
                temp.push_back(*start);
                ++start;
            }

            if (temp.size() >= 1)
                m_xpKill = std::stoi(temp[temp.size() - 1]);
            else
                m_xpKill = 0;

            m_xpTotal += m_xpKill;
        } else if (std::regex_search(data, exemplar_chain))
        {
            std::sregex_token_iterator start(data.begin(), data.end(), get_number, 0), end;

            while (start != end)
            {
                temp.push_back(*start);
                ++start;
            }

            if (temp.size() >= 1)
                m_exemplarKill = std::stoi(temp[temp.size() - 1]);
            else
                m_exemplarKill = 0;

            m_exemplarTotal += m_exemplarKill;
        }
    }
    if (sMode == 131)
    {
        if (std::regex_search(data, xp_inc))
        {
            std::sregex_token_iterator start(data.begin(), data.end(), get_number, 0), end;
            temp.clear();

            while (start != end)
            {
                temp.push_back(*start);
                ++start;
            }

            if (temp.size() >= 1)
                m_xpKill = std::stoi(temp[temp.size() - 1]);
            else
                m_xpKill = 0;

            m_xpTotal += m_xpKill;
        } else if (std::regex_search(data, exemplar_chain))
        {
            std::sregex_token_iterator start(data.begin(), data.end(), get_number, 0), end;

            while (start != end)
            {
                temp.push_back(*start);
                ++start;
            }

            if (temp.size() >= 1)
                m_exemplarKill = std::stoi(temp[temp.size() - 1]);
            else
                m_exemplarKill = 0;

            m_exemplarTotal += m_exemplarKill;
        }
    }
    else if (sMode == 633)
    {
        if (std::regex_search(data, cru_inc))
        {
            std::sregex_token_iterator start(data.begin(), data.end(), get_number, 0), end;
            temp.clear();

            while (start != end)
            {
                temp.push_back(*start);
                ++start;
            }

            if (temp.size() >= 1)
                m_crKill = std::stoi(temp[temp.size() - 1]);
            else
                m_crKill = 0;

            m_crTotal += m_crKill;
        }
        else if (std::regex_search(data, vis_inc))
        {
            m_vTime += 10;
        }
    }
    else if (sMode == 653)
    {
        if ((std::regex_search(data, dyn_expelled)) && (std::regex_search(data, dyn_extend)))
        {
            std::regex_search(data, m, get_number);

            m_DynaTime = std::stoi(m.str());

            m_DynTimeStart = time(nullptr);
        }
    }
    else if (sMode == 658)
    {
        if ((std::regex_search(data, dyn_extend)) && (std::regex_search(data, dyn_extend2)))
        {
            std::regex_search(data, m, get_number);

            m_DynaTime += std::stoi(m.str());

            m_DynTimeStart = time(nullptr);
        }
    }
    else if (sMode == 660)
    {
        if ((std::regex_search(data, dyn_start)) && (std::regex_search(data, dyn_start2)))
        {
            std::regex_search(data, m, get_number);

            m_DynaTime = std::stoi(m.str());
        }
        if (std::regex_search(data, vis_exit))
        {
            std::regex_search(data, m, get_number);

            m_vTime = std::stoi(m.str());
        }
        else if (std::regex_search(data, cru_inc))
        {
            std::regex_search(data, m, get_number);
            m_crKill = std::stoi(m.str());

            m_crTotal += m_crKill;
        }
        else if (std::regex_search(data, vis_wear))
        {
            std::regex_search(data, m, get_number);
            m_vTime = std::stoi(m.str());

            m_AbyTimeStart = time(nullptr);
        }
        else if (std::regex_search(data, vis_pegs))
        {
            std::sregex_token_iterator start(data.begin(), data.end(), get_number, 0), end;

            while (start != end)
            {
                temp.push_back(*start);
                ++start;
            }

            //Corrects Pearlescent Values
            m_prl = std::stoi(temp[0]);

            //Corrects Ebon Values
            m_ebn = std::stoi(temp[1]);

            //Corrects Gold Values
            m_gld = std::stoi(temp[2]);

            //Corrects Silvery Values
            m_slv = std::stoi(temp[3]);

        }
        else if (std::regex_search(data, vis_azram))
        {
            std::sregex_token_iterator start(data.begin(), data.end(), get_number, 0), end;

            while (start != end)
            {
                temp.push_back(*start);
                ++start;
            }

            //Corrects Azure Values
            m_azr = std::stoi(temp[0]);

            //Corrects Ruby Values
            m_rby = std::stoi(temp[1]);

            //Corrects Amber Values
            m_amb = std::stoi(temp[2]);

        }
        else if (std::regex_search(data, light_pearl))
        {
            //checks to ensure value does not exceed 230
            if (m_prl + 5 > 230)
            {
                m_prl = 230;
            }
            else
            {
                m_prl += 5;
            }
        }
        else if (std::regex_search(data, light_gold))
        {
            //checks to ensure value does not exceed 200
            if (m_gld + 5 > 200)
            {
                m_gld = 200;
            }
            else
            {
                m_gld += 5;
            }
        }
        else if (std::regex_search(data, light_silver))
        {
            //checks to ensure value does not exceed 200
            if (m_slv + 5 > 200)
            {
                m_slv = 200;
            }
            else
            {
                m_slv += 5;
            }
        }
        else if (std::regex_search(data, light_ebon))
        {
            //checks to ensure value does not exceed 200
            if (m_ebn + 1 > 200)
            {
                m_ebn = 200;
            }
            else
            {
                m_ebn += 1;
            }
        }
        else if (std::regex_search(data, light_azure))
        {
            //checks to ensure value does not exceed 255
            if (m_azr + 8 > 255)
            {
                m_azr = 255;
            }
            else
            {
                m_azr += 8;
            }
        }
        else if (std::regex_search(data, light_ruby))
        {
            //checks to ensure value does not exceed 255
            if (m_rby + 8 > 255)
            {
                m_rby = 255;
            }
            else
            {
                m_rby += 8;
            }
        }
        else if (std::regex_search(data, light_amber))
        {
            //checks to ensure value does not exceed 255
            if (m_amb + 8 > 255)
            {
                m_amb = 255;
            }
            else
            {
                m_amb += 8;
            }
        }
    }
#pragma endregion

    return false;
}

bool WatchEXP::Direct3DInitialize(IDirect3DDevice8* lpDevice)
{
    this->m_Direct3DDevice = lpDevice;

    m_ExpObj = m_AshitaCore->GetFontManager()->Create("EXP_ID");
    m_ExpObj->SetText("0/0");
    m_ExpObj->SetFontFamily(font_name);
    m_ExpObj->SetFontHeight(font_size);
    m_ExpObj->SetColor(0xFFFFFFFF);
    m_ExpObj->SetBold(false);
    m_ExpObj->SetVisibility(true);
    m_ExpObj->SetPositionX(100);
    m_ExpObj->SetPositionY(100);
    m_ExpObj->SetVisibility(false);
    
    m_ExpObj->GetBackground()->SetVisibility(true);
    m_ExpObj->GetBackground()->SetColor(0x80000000);

    m_BaseTimeEXP = time(nullptr);
    m_BaseTimeCR = time(nullptr);
    m_BaseTimeExemplar = time(nullptr);

    return true;
}

void WatchEXP::Direct3DRelease(void)
{
    m_AshitaCore->GetFontManager()->Delete("EXP_ID");
}

void WatchEXP::Direct3DRender(void)
{
    doEXPCalculations();
    doAbysseaZoneTimings();
    doDynamisZoneTimings();
    checkResetTime();
}

__declspec(dllexport) double __stdcall GetInterfaceVersion(void)
{
    return ASHITA_INTERFACE_VERSION;
}

__declspec(dllexport) void __stdcall CreatePluginInfo(plugininfo_t* info)
{
    g_PluginInformation = info;

    strcpy_s(info->Author, sizeof(info->Author), "Praenuntiae (ported by Vicrelant)");
    strcpy_s(info->Name, sizeof(info->Name), "WatchEXP");
    info->InterfaceVersion = ASHITA_INTERFACE_VERSION;
    info->PluginVersion = 3.1f;
    info->Priority = 0;
}

__declspec(dllexport) IPlugin* __stdcall CreatePlugin(void)
{
    return (IPlugin*)new WatchEXP();
}

#pragma region GetConfig
void WatchEXP::GetConfig()
{
    auto iConfigResult = false;
    auto iTemp = 0;

    iConfigResult = m_AshitaCore->GetConfigurationManager()->Load("WatchEXP", "WatchEXP");

    if (iConfigResult != 0)
    {
        auto szBuffer = m_AshitaCore->GetConfigurationManager()->get_string("WatchEXP", "font_name");
        strcpy_s(font_name, szBuffer);

        font_size = m_AshitaCore->GetConfigurationManager()->get_int32("WatchEXP", "font_size", 11);
        show_XP_M_always = m_AshitaCore->GetConfigurationManager()->get_bool("WatchEXP", "show_both_xp_and_merits_when_capped", false);
        m_reset_min = m_AshitaCore->GetConfigurationManager()->get_int32("WatchEXP", "auto_reset_time", 45);
        m_autoreset = m_AshitaCore->GetConfigurationManager()->get_bool("WatchEXP", "auto_reset_enabled", false);
        m_exemplarMode = m_AshitaCore->GetConfigurationManager()->get_bool("WatchEXP", "exemplar_mode", false);
    }
    else if (iConfigResult == 0)
    {
        m_AshitaCore->GetConfigurationManager()->set_value("WatchEXP", "font_name", "Arial");
        m_AshitaCore->GetConfigurationManager()->set_value("WatchEXP", "font_size", "11");
        m_AshitaCore->GetConfigurationManager()->set_value("WatchEXP", "show_both_xp_and_merits_when_capped", "true");
        m_AshitaCore->GetConfigurationManager()->set_value("WatchEXP", "auto_reset_time", "45");
        m_AshitaCore->GetConfigurationManager()->set_value("WatchEXP", "auto_reset_enabled", "false");
        m_AshitaCore->GetConfigurationManager()->set_value("WatchEXP", "exemplar_mode", "false");
        m_AshitaCore->GetConfigurationManager()->Save("WatchEXP", "WatchEXP");

        strcpy_s(font_name, "Arial");
        font_size = 11;
        show_XP_M_always = false;
        m_reset_min = 45;
        m_autoreset = false;
        m_exemplarMode = false;
    }
}
#pragma endregion

#pragma region doEXPCalculations
void WatchEXP::doEXPCalculations()
{
    char *p;
    char expIntoLVL[10] = "", expForLVL[10] = "", expTillLVL[10] = "", lmtIntoLVL[10] = "", lmtTillLVL[10] = "", xpTotal[40] = "", crTotal[40] = "";
    char exemplarIntoLVL[10] = "", exemplarForLVL[10] = "", exemplarTillLVL[10] = "", exemplarTotal[40] = "";

    m_IsInAbyssea = isInAbyssea(m_AshitaCore->GetDataManager()->GetParty()->GetMemberZone(0));
    m_IsInDynamis = isInDynamis(m_AshitaCore->GetDataManager()->GetParty()->GetMemberZone(0));

    // get the EXP info
    unsigned short expIN = m_AshitaCore->GetDataManager()->GetPlayer()->GetExpCurrent();
    unsigned short expMAX = m_AshitaCore->GetDataManager()->GetPlayer()->GetExpNeeded();
    unsigned short expLEFT = expMAX - expIN;

    // get Exemplar/Master level info
    unsigned long exemplarIN = m_AshitaCore->GetDataManager()->GetPlayer()->GetMasteryExp();
    unsigned long exemplarMAX = m_AshitaCore->GetDataManager()->GetPlayer()->GetMasteryExpNeeded();
    unsigned long exemplarLEFT = exemplarMAX - exemplarIN;

    // get the Merit info
    unsigned short lmtIN = m_AshitaCore->GetDataManager()->GetPlayer()->GetLimitPoints();
    unsigned short lmtMAX = 10000;
    unsigned short lmtLEFT = lmtMAX - lmtIN;

    p = expIntoLVL;
    makeComma(expIN, p, 0);
    _strrev(expIntoLVL);

    p = expForLVL;
    makeComma(expMAX, p, 0);
    _strrev(expForLVL);

    p = expTillLVL;
    makeComma(expLEFT, p, 0);
    _strrev(expTillLVL);

    p = lmtIntoLVL;
    makeComma(lmtIN, p, 0);
    _strrev(lmtIntoLVL);

    p = lmtTillLVL;
    makeComma(lmtLEFT, p, 0);
    _strrev(lmtTillLVL);

    p = xpTotal;
    makeComma(m_xpTotal, p, 0);
    _strrev(xpTotal);

    p = crTotal;
    makeComma(m_crTotal, p, 0);
    _strrev(crTotal);

    p = exemplarIntoLVL;
    makeComma(exemplarIN, p, 0);
    _strrev(exemplarIntoLVL);

    p = exemplarForLVL;
    makeComma(exemplarMAX, p, 0);
    _strrev(exemplarForLVL);

    p = exemplarTillLVL;
    makeComma(exemplarLEFT, p, 0);
    _strrev(exemplarTillLVL);

    p = exemplarTotal;
    makeComma(m_exemplarTotal, p, 0);
    _strrev(exemplarTotal);

    if (m_AshitaCore->GetDataManager()->GetPlayer()->GetMainJobLevel() < 75)
    {
        if (m_IsInDynamis)
        {
            sprintf_s(TextObjectBuffer, MAX_PATH, " %s/%s(%s)  XP:%s(%u) %.1fk/hr  Alb:%c Amb:%c Azr:%c Crm:%c Obn:%c  Time Remaining: %d ", expIntoLVL, expForLVL, expTillLVL,
                xpTotal, m_xpKill, returnProjectedEXPperHour(m_xpTotal), m_Dyna_Alab, m_Dyna_Amb, m_Dyna_Azr, m_Dyna_Crm, m_Dyna_Obs, m_DynaTime);
        }
        else if (m_IsInAbyssea)
        {
            sprintf_s(TextObjectBuffer, MAX_PATH, " %s/%s(%s)  XP:%s(%u) %.1fk/hr  Cruor:%s(%u) %.1fk/hr  Prl:%u Gld:%u Slv:%u Ebn:%u Azr:%u Rby:%u Amb:%u  V.Time: %d ",
                expIntoLVL, expForLVL, expTillLVL, xpTotal, m_xpKill, returnProjectedEXPperHour(m_xpTotal), crTotal, m_crKill, returnProjectedCRUORperHour(m_crTotal),
                m_prl, m_gld, m_slv, m_ebn, m_azr, m_rby, m_amb, m_vTime);
        }
        else
        {
            sprintf_s(TextObjectBuffer, MAX_PATH, " %s/%s(%s)  XP:%s(%u) %.1fk/hr ", expIntoLVL, expForLVL, expTillLVL,
                xpTotal, m_xpKill, returnProjectedEXPperHour(m_xpTotal));
        }
    }
    if ((!show_XP_M_always) && (m_AshitaCore->GetDataManager()->GetPlayer()->GetMainJobLevel() >= 75) && (!m_exemplarMode))
    {
        unsigned char isMeritMode = m_AshitaCore->GetDataManager()->GetPlayer()->GetLimitMode();

        if ((expLEFT == 1) || (isMeritMode == 224))
        {
            if (m_IsInDynamis)
            {
                sprintf_s(TextObjectBuffer, MAX_PATH, " %s/10,000(%s)[%i]  XP:%s(%u) %.1fk/hr  Alb:%c Amb:%c Azr:%c Crm:%c Obn:%c  Time Remaining: %d ", lmtIntoLVL, lmtTillLVL,
                    (int)m_AshitaCore->GetDataManager()->GetPlayer()->GetMeritPoints(), xpTotal, m_xpKill, returnProjectedEXPperHour(m_xpTotal), m_Dyna_Alab, m_Dyna_Amb, m_Dyna_Azr, m_Dyna_Crm,
                    m_Dyna_Obs, m_DynaTime);
            }
            else if (m_IsInAbyssea)
            {
                sprintf_s(TextObjectBuffer, MAX_PATH, " %s/10,000(%s)[%i]  XP:%s(%u) %.1fk/hr  Cruor:%s(%u) %.1fk/hr  Prl:%u Gld:%u Slv:%u Ebn:%u Azr:%u Rby:%u Amb:%u  V.Time: %d ", lmtIntoLVL, lmtTillLVL,
                    (int)m_AshitaCore->GetDataManager()->GetPlayer()->GetMeritPoints(), xpTotal, m_xpKill, returnProjectedEXPperHour(m_xpTotal), crTotal, m_crKill, returnProjectedCRUORperHour(m_crTotal),
                    m_prl, m_gld, m_slv, m_ebn, m_azr, m_rby, m_amb, m_vTime);
            }
            else
            {
                sprintf_s(TextObjectBuffer, MAX_PATH, " %s/10,000(%s)[%i]  XP:%s(%u) %.1fk/hr ", 
                    lmtIntoLVL, lmtTillLVL, (int)m_AshitaCore->GetDataManager()->GetPlayer()->GetMeritPoints(), xpTotal,
                    m_xpKill, returnProjectedEXPperHour(m_xpTotal));
            }
        }
        else
        {
            if (m_IsInDynamis)
            {
                sprintf_s(TextObjectBuffer, MAX_PATH, " %s/%s(%s)  XP:%s(%u) %.1fk/hr  Alb:%c Amb:%c Azr:%c Crm:%c Obn:%c  Time Remaining: %d ", expIntoLVL, expForLVL, expTillLVL,
                    xpTotal, m_xpKill, returnProjectedEXPperHour(m_xpTotal), m_Dyna_Alab, m_Dyna_Amb, m_Dyna_Azr, m_Dyna_Crm, m_Dyna_Obs, m_DynaTime);
            }
            else if (m_IsInAbyssea)
            {
                sprintf_s(TextObjectBuffer, MAX_PATH, " %s/%s(%s)  XP:%s(%u) %.1fk/hr  Cruor:%s(%u) %.1fk/hr  Prl:%u Gld:%u Slv:%u Ebn:%u Azr:%u Rby:%u Amb:%u  V.Time: %d ",
                    expIntoLVL, expForLVL, expTillLVL, xpTotal, m_xpKill, returnProjectedEXPperHour(m_xpTotal), crTotal, m_crKill, returnProjectedCRUORperHour(m_crTotal), m_prl,
                    m_gld, m_slv, m_ebn, m_azr, m_rby, m_amb, m_vTime);
            }
            else
            {
                sprintf_s(TextObjectBuffer, MAX_PATH, " %s/%s(%s)  XP:%s(%u) %.1fk/hr ", expIntoLVL, expForLVL, expTillLVL, xpTotal, m_xpKill, returnProjectedEXPperHour(m_xpTotal));
            }
        }
    }
    if ((show_XP_M_always) && (m_AshitaCore->GetDataManager()->GetPlayer()->GetMainJobLevel() >= 75) && (!m_exemplarMode))
    {
        if (m_IsInDynamis)
        {
            sprintf_s(TextObjectBuffer, MAX_PATH, " %s/%s(%s)  %s/10,000(%s)[%i]  XP:%s(%u) %.1fk/hr  Alb:%c Amb:%c Azr:%c Crm:%c Obn:%c  Time Remaining: %d ", expIntoLVL, expForLVL, expTillLVL,
                lmtIntoLVL, lmtTillLVL, (int)m_AshitaCore->GetDataManager()->GetPlayer()->GetMeritPoints(), xpTotal, m_xpKill, returnProjectedEXPperHour(m_xpTotal), m_Dyna_Alab, m_Dyna_Amb,
                m_Dyna_Azr, m_Dyna_Crm, m_Dyna_Obs, m_DynaTime);
        }
        else if (m_IsInAbyssea)
        {
            sprintf_s(TextObjectBuffer, MAX_PATH, " %s/%s(%s)  %s/10,000(%s)[%i]  XP:%s(%u) %.1fk/hr  Cruor:%s(%u) %.1fk/hr  Prl:%u Gld:%u Slv:%u Ebn:%u Azr:%u Rby:%u Amb:%u  V.Time: %d ",
                expIntoLVL, expForLVL, expTillLVL, lmtIntoLVL, lmtTillLVL, (int)m_AshitaCore->GetDataManager()->GetPlayer()->GetMeritPoints(), xpTotal, m_xpKill, returnProjectedEXPperHour(m_xpTotal),
                crTotal, m_crKill, returnProjectedCRUORperHour(m_crTotal), m_prl, m_gld, m_slv, m_ebn, m_azr, m_rby, m_amb, m_vTime);
        }
        else
        {
            sprintf_s(TextObjectBuffer, MAX_PATH, " %s/%s(%s)  %s/10,000(%s)[%i]  XP:%s(%u) %.1fk/hr ", expIntoLVL, expForLVL, expTillLVL,
                lmtIntoLVL, lmtTillLVL, (int)m_AshitaCore->GetDataManager()->GetPlayer()->GetMeritPoints(), xpTotal, m_xpKill, returnProjectedEXPperHour(m_xpTotal));
        }
    }
    if (m_exemplarMode) {
        sprintf_s(TextObjectBuffer, MAX_PATH, " %s/%s(%s)    Exemplar:%s(%u) %.1fk/hr ", exemplarIntoLVL, exemplarForLVL, exemplarTillLVL, exemplarTotal, m_exemplarKill, returnProjectedExemplarPerHour(m_exemplarTotal));
    }

    // double check the size of the text
    if (m_ExpObjSize.cy == 0)
    {
        int window_y = m_AshitaCore->GetConfigurationManager()->get_int32("boot_config", "window_y", 600);
        m_ExpObj->GetTextSize(&m_ExpObjSize);
        m_ExpObj->SetPositionX(15.0f);
        m_ExpObj->SetPositionY((float)(window_y - m_ExpObjSize.cy));
    }

    // set the object information
    m_ExpObj->SetText(TextObjectBuffer);
    m_ExpObj->SetVisibility(true);
}
#pragma endregion
#pragma region returnProjectedEXPperHour
float WatchEXP::returnProjectedEXPperHour(unsigned int expIntoLevel)
{
    float changeInTime = (float(time(nullptr) - m_BaseTimeEXP));
    float changeInTimeHR = ((changeInTime / 60) / 60);

    if (expIntoLevel == 0)
    {
        m_BaseTimeEXP = time(nullptr);
    }

    // just in case we don't do a divide by zero
    if (changeInTime == 0.0f)
    {
        return 0;
    }

    return ((((float)expIntoLevel) / changeInTimeHR) / 1000);
}
#pragma endregion
#pragma region returnProjectedExemplarPerHour
float WatchEXP::returnProjectedExemplarPerHour(unsigned int exemplarIntoLevel)
{
    float changeInTime = (float(time(nullptr) - m_BaseTimeExemplar));
    float changeInTimeHR = ((changeInTime / 60) / 60);

    if (exemplarIntoLevel == 0)
    {
        m_BaseTimeExemplar = time(nullptr);
    }

    // just in case we don't do a divide by zero
    if (changeInTime == 0.0f)
    {
        return 0;
    }

    return ((((float)exemplarIntoLevel) / changeInTimeHR) / 1000);
}
#pragma endregion
#pragma region returnProjectedCRUORperHour
float WatchEXP::returnProjectedCRUORperHour(unsigned int cruorLVL)
{
    float changeInTime = (float(time(nullptr) - m_BaseTimeCR));
    float changeInTimeHR = ((changeInTime / 60) / 60);

    if (cruorLVL == 0)
    {
        m_BaseTimeCR = time(nullptr);
    }

    // just in case we don't do a divide by zero
    if (changeInTime == 0.0f)
    {
        return 0;
    }

    return ((((float)cruorLVL) / changeInTimeHR) / 1000);
}
#pragma endregion
#pragma region isInAbyssea
bool WatchEXP::isInAbyssea(int zoneID)
{
    // look for it
    for (auto& x : AbysseaZoneIDs)
    {
        // Yes!
        if (x == zoneID)
            return true;
    }

    m_prl = 0, m_gld = 0, m_amb = 0, m_slv = 0, m_azr = 0, m_ebn = 0, m_rby = 0, m_vTime = 0;

    return false;
}
#pragma endregion
#pragma region isInDynamis
bool WatchEXP::isInDynamis(int zoneID)
{
    // look for it
    for (auto& x : DynamisZoneIDs)
    {
        // Yes!
        if (x == zoneID)
            return true;
    }

    m_Dyna_Crm = 'X', m_Dyna_Azr = 'X', m_Dyna_Alab = 'X', m_Dyna_Amb = 'X', m_Dyna_Obs = 'X';
    // Nope
    return false;
}
#pragma endregion
#pragma region doAbysseaZoneTimings
void WatchEXP::doAbysseaZoneTimings()
{
    // check to see if we are in Abyssea
    m_IsInAbyssea = isInAbyssea(m_AshitaCore->GetDataManager()->GetParty()->GetMemberZone(0));

    if (m_IsInAbyssea && (time(nullptr) - m_AbyTimeStart) > 60)
    {
        m_AbyTimeStart = time(nullptr);
        if (m_vTime != 0)
        {
            m_vTime -= 1;
        }
    }
}
#pragma endregion
#pragma region doDynamisZoneTimings
void WatchEXP::doDynamisZoneTimings()
{
    // check to see if we are in Abyssea
    m_IsInDynamis = isInDynamis(m_AshitaCore->GetDataManager()->GetParty()->GetMemberZone(0));

    doDynamisKItemCheck();

    if (m_IsInDynamis && (time(nullptr) - m_DynTimeStart) > 60)
    {
        m_DynTimeStart = time(nullptr);
        if (m_DynaTime != 0)
        {
            m_DynaTime -= 1;
        }
    }
}
#pragma endregion
#pragma region doDynamisKItemCheck
void WatchEXP::doDynamisKItemCheck()
{
    //Checks for Crimson Granule of Time

    if (m_AshitaCore->GetDataManager()->GetPlayer()->HasKeyItem(1545))
    {
        m_Dyna_Crm = 'O';
    }
    //Checks for Azure Granule of Time
    if (m_AshitaCore->GetDataManager()->GetPlayer()->HasKeyItem(1546))
    {
        m_Dyna_Azr = 'O';
    }
    //Checks for Amber Granule of Time
    if (m_AshitaCore->GetDataManager()->GetPlayer()->HasKeyItem(1547))
    {
        m_Dyna_Amb = 'O';
    }
    //Checks for Alabaster Granule of Time
    if (m_AshitaCore->GetDataManager()->GetPlayer()->HasKeyItem(1548))
    {
        m_Dyna_Alab = 'O';
    }
    //Checks for Obsidian Granule of Time
    if (m_AshitaCore->GetDataManager()->GetPlayer()->HasKeyItem(1549))
    {
        m_Dyna_Obs = 'O';
    }
}
#pragma endregion
#pragma region makeComma
void WatchEXP::makeComma(int n, char *p, int count)
{
    int d;
    // Get least significant digit
    d = n % 10;
    // Convert to ascii character and store
    *p++ = d + '0';
    // Remove least significant digit
    // Test for finished
    if (n /= 10)
    {
        // Not finished, increase digit count,
        // test for comma position
        count++;
        if (count >= 3)
        {
            // Add the comma and reset the count
            *p++ = ',';
            count = 0;
        }
        // Call this procedure to work on
        // remaining part of number
        makeComma(n, p, count);
    }
    return;
}
#pragma endregion

#pragma region convertFromHex
int WatchEXP::convertFromHex(std::string hex)
{
    int value = 0;
    int a = 0;
    int b = hex.length() - 1;

    for (; b >= 0; a++, b--)
    {
        if (hex[b] >= '0' && hex[b] <= '9')
        {
            value += (hex[b] - '0') * (1 << (a * 4));
        }
        else
        {
            switch (hex[b])
            {
            case 'A':

            case 'a':
                value += 10 * (1 << (a * 4));
                break;

            case 'B':

            case 'b':
                value += 11 * (1 << (a * 4));
                break;

            case 'C':

            case 'c':
                value += 12 * (1 << (a * 4));
                break;

            case 'D':

            case 'd':
                value += 13 * (1 << (a * 4));
                break;

            case 'E':

            case 'e':
                value += 14 * (1 << (a * 4));
                break;

            case 'F':

            case 'f':
                value += 15 * (1 << (a * 4));
                break;

            default:

                break;

            }
        }
    }
    return value;
}
#pragma endregion
#pragma region hextodec
void WatchEXP::hextodec(std::string hex, std::vector<unsigned char>& rgb)
{
    std::string redString, greenString, blueString;

    if (hex.compare(0, 1, "#") == 0)
    {
        //If the prefix # was attached to hex, use the following code
        redString = hex.substr(1, 2);
        greenString = hex.substr(3, 2);
        blueString = hex.substr(5, 2);
    }
    else if (hex.compare(0, 2, "0x") == 0)
    {
        //If the prefix 0x was attached to hex, use the following code
        redString = hex.substr(2, 2);
        greenString = hex.substr(4, 2);
        blueString = hex.substr(6, 2);
    }
    else
    {
        //If there is no prefix attached to hex, use this code
        redString = hex.substr(0, 2);
        greenString = hex.substr(2, 2);
        blueString = hex.substr(4, 2);
    }

    unsigned char red = (unsigned char)(convertFromHex(redString));
    unsigned char green = (unsigned char)(convertFromHex(greenString));
    unsigned char blue = (unsigned char)(convertFromHex(blueString));

    rgb[0] = red;
    rgb[1] = green;
    rgb[2] = blue;
}
#pragma endregion
#pragma region convertTrans
void WatchEXP::convertTrans(unsigned char& trans)
{
    float temp1, temp2;

    temp1 = (float)trans;

    temp2 = temp1 * (float)2.55;

    if (temp2 > 255)
    {
        trans = (unsigned char)255;
    }
    else
    {
        trans = (unsigned char)temp2;
    }
}
#pragma endregion
#pragma region checkResetTime
void WatchEXP::checkResetTime()
{
    if (time(nullptr) - m_resetTime > 60)
    {
        if (m_xpTest != m_xpTotal)
        {
            m_xpTest = m_xpTotal;
            m_resetTime = time(nullptr);
        }
        else if ((m_xpTest == m_xpTotal) && (m_autoreset))
        {
            if (((time(nullptr) - m_resetTime) / 60) >= m_reset_min)
            {
                m_resetTime = time(nullptr);

                m_crTotal = 0, m_xpTotal = 0, m_xpKill = 0, m_crKill = 0, m_exemplarTotal = 0, m_exemplarKill = 0;
            }
        }
    }
}
#pragma endregion
#pragma region ScrubChat
std::string WatchEXP::ScrubChat(const char* chat_line)
{
    //strings to hold the chat data through the various scrubbing stages
    std::string chat_raw;
    std::string clean_chat_pass1;
    std::string clean_chat_pass2;
    std::string clean_chat_pass3;
    std::string clean_chat_final;

    char szClean[1024];

    //regex expressions for parsing the raw chat until it's clean
    std::regex parse1("\\x1E\\x05|\\x1E(\\x03|\\x02|\\x01)|\\x7F\\x31|\\x1F(\\x79|\\x7F|\\x7B|\\W)");
    std::regex parse2("[[:cntrl:]]|\\W\\W");
    std::regex time_stamp("\\x1E\\x03((\\[\\d+(:|/)\\d+(:|/)\\d+(\\]| [AMP]+\\]))|\\[\\d+(:|/)\\d+(:|/)\\d+ - \\d+:\\d+:\\d+(\\]| [AMP]+\\]))\\x1E\\x01\\s+");
    std::regex multi_space("\\s+");

    memset(szClean, 0, 1024);

    //Cleans up any auto translate data so it can be properly parsed
    m_AshitaCore->GetChatManager()->ParseAutoTranslate(chat_line, szClean, 1024, false);

    chat_raw = chat_line;

    //Removes TimeStamp data from beginning of line
    std::regex_replace(std::back_inserter(clean_chat_pass1), chat_raw.begin(), chat_raw.end(), time_stamp, "");

    //Removes some control characters from the beginning and end of the string that print as garbage
    std::regex_replace(std::back_inserter(clean_chat_pass2), clean_chat_pass1.begin(), clean_chat_pass1.end(), parse1, "");

    //Removes any remaining control characters and replaces them with spaces
    std::regex_replace(std::back_inserter(clean_chat_pass3), clean_chat_pass2.begin(), clean_chat_pass2.end(), parse2, " ");

    //Changes any multi space areas to single spaces
    std::regex_replace(std::back_inserter(clean_chat_final), clean_chat_pass3.begin(), clean_chat_pass3.end(), multi_space, " ");

    return clean_chat_final;
}
#pragma endregion
