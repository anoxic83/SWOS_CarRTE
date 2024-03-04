#include "anxswos.h"
#define SWSLOG_IMPLEMENTATION
#include "swslog.h"
//Init vars
const uintptr_t ptrEnhancement = 0xA596E0 - 0x400000; // Is OpenGL or SDL
const uintptr_t ptrSDLWindow = 0x4BF82A8 - 0x400000;
const uintptr_t ptrSDLRenderer = 0xA59888 - 0x400000;
const uintptr_t ptrGLContext = 0xA59884 - 0x400000;

const uintptr_t ptrWindowWidth = 0x4EF9C94 - 0x400000;
const uintptr_t ptrWindowHeight = 0x4EF9C98 - 0x400000;

// Helpers
const uintptr_t ptrInputingText = 0x54FDA73 - 0x400000;

// Career 
// 54FE068
const uintptr_t ptrGameType = 0x20068;  // + DSeg
//54FC068
//old : 0x547B078 diff: 80FF0
const uintptr_t ptrCareerFileBuffer = 0x000095a6; // + DSEG
// 
const uintptr_t ptrNewBalance = ptrCareerFileBuffer + 0xd5dc;
const uintptr_t ptrCareerTeam = ptrCareerFileBuffer + 0xdb80;
const uintptr_t ptrCarTmPlayerCount = ptrCareerFileBuffer + 0xe08c;
const uintptr_t ptrCareerTransfers = ptrCareerFileBuffer + 0x125a8;
const uintptr_t ptrCareerPlayPool = ptrCareerFileBuffer + 0xD876;
const uintptr_t ptrCareerManager = ptrCareerFileBuffer + 0xD74A;
const uintptr_t ptrJobOffers = ptrCareerFileBuffer + 0xd908;

const uintptr_t ptrCompetitionTables = 0x8B36; // + Dseg;

AnxSWOS::AnxSWOS(uintptr_t base, bool overlay)
: m_Base(base), m_Window(nullptr), m_Renderer(nullptr), m_ImGuiCtx(nullptr), m_GUIOverlay(overlay)
{
  log_init(LOG_LEV_INFO, "plugins/swos_car_rte.log");
  log_info("[ANXSWOS] => Plugin loaded.");
  log_info("[ANXSWOS] => SWOS Base address: 0x%p", m_Base);
  m_CurrentHexAddress = m_Base;
  m_DataSize = 0xffff;
  m_HexMemory = false;
  m_GUIEnabled = false;
  m_DSeg = SWOSHook::GetDSegDataPtr();
}

AnxSWOS::~AnxSWOS()
{
  if (m_OpenGLRenderer)
    ImGui_ImplOpenGL3_Shutdown();
  else
    ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
  log_info("[ANXSWOS] => Plugin unloaded.");
}

SDL_Texture* AnxSWOS::loadSDLTexture(const char* path)
{
  // Copy from https://lazyfoo.net/tutorials/SDL/07_texture_loading_and_rendering/index.php
  //The final texture
  SDL_Texture* newTexture = NULL;

  //Load image at specified path
  SDL_Surface* loadedSurface = SDL_LoadBMP( path );
  if( loadedSurface == NULL )
  {
    log_error( "[AnxSWOS] => Unable to load image %s! SDL_image Error: %s\n", path, SDL_GetError() );
  }
  else
  {
    //Create texture from surface pixels
    newTexture = SDL_CreateTextureFromSurface( m_Renderer, loadedSurface );
    if( newTexture == NULL )
    {
        log_error( "[AnxSWOS] => Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError() );
    }
    //Get rid of old loaded surface
    SDL_FreeSurface( loadedSurface );
  }

  return newTexture;
}

bool AnxSWOS::IsCareer()
{
  int16_t gameType = 0;
  SWOSHook::ReadMemory(ptrGameType + m_DSeg, &gameType, 2);
  return (gameType == 4);
}

void AnxSWOS::Init()
{
  SWOSHook::ReadMemory(ptrEnhancement + m_Base, &m_OpenGLRenderer, 4);
  uint32_t wnd = 0;
  SWOSHook::ReadMemory(ptrSDLWindow + m_Base, &wnd, 4);
  m_Window = (SDL_Window*)wnd;
  if (m_OpenGLRenderer)
  {
    SWOSHook::ReadMemory(ptrGLContext + m_Base, &m_GLContext, 4);
  }
  else
  {
    uint32_t ren = 0;
    SWOSHook::ReadMemory(ptrSDLRenderer + m_Base, &ren, 4);
    m_Renderer = (SDL_Renderer*)ren;
  }
  IMGUI_CHECKVERSION();
  m_ImGuiCtx = ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  ImGui::StyleColorsDark();


  ImGuiStyle& style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
      style.WindowRounding = 0.0f;
      style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  // Setup Platform/Renderer backends
  if (m_OpenGLRenderer)
  {
    ImGui_ImplSDL2_InitForOpenGL(m_Window, m_GLContext);
    ImGui_ImplOpenGL3_Init("#version 130");
  }
  else
  {
    ImGui_ImplSDL2_InitForSDLRenderer(m_Window, m_Renderer);
    ImGui_ImplSDLRenderer2_Init(m_Renderer);
  }

}

void AnxSWOS::OnEvent(SDL_Event* e)
{
  if (m_ImGuiCtx != nullptr)
  {
    ImGui_ImplSDL2_ProcessEvent(e);
    if (e->type == SDL_KEYDOWN)
    {

      if ((e->key.keysym.sym == SDLK_e)&&(e->key.keysym.mod & KMOD_LCTRL) != 0)
      {
        m_GUIEnabled = !m_GUIEnabled;
      }
    }
  }
}

void AnxSWOS::Draw()
{

  int width;
  int heigth;
  SDL_GetWindowSize(m_Window, &width, &heigth);
  
  if (m_OpenGLRenderer)
    ImGui_ImplOpenGL3_NewFrame();
  else
  {
    ImGui_ImplSDLRenderer2_NewFrame();
  }
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  if (m_GUIEnabled)
  {

    //DRAW
    ImVec2 pos = ImGui::GetMainViewport()->Pos;
    ImVec2 size = ImGui::GetMainViewport()->Size;


    ImGui::SetNextWindowBgAlpha(0.4f);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(width * 0.2f, heigth * 0.2f), ImGuiCond_FirstUseEver);
    //const std::string imwinname = (m_GUIOverlay) ? "GUI Plugin [OvelayMode]" : "GUI Plugin [Override Mode]";
    ImGui::Begin("Anx-SWOS-CarRTE 0.1.3b");
    if (IsCareer())
    {
      ImGui::Text("base_ptr: 0x%08x", m_Base);
      ImGui::Text("manager_ptr: 0x%08x", m_DSeg+ptrCareerManager);
      ImGui::Text("team_ptr: 0x%08x", m_DSeg+ptrCareerTeam);
      ImGui::Text("career_ptr: 0x%08x", m_DSeg+ptrCareerFileBuffer);

      //SWOSHook::ReadMemory(m_Base + ptrCareerManager, &m_Manager, sizeof(ASWSCarManager));
      SWOSHook::ReadMemory(m_DSeg + ptrCareerTeam, &m_MainTeam, sizeof(ASWSTeamCar));
      SWOSHook::ReadMemory(ptrCareerManager + m_DSeg, &m_Manager, sizeof(ASWSCarManager));
      SWOSHook::ReadMemory(ptrCarTmPlayerCount + m_DSeg, &m_PlayersCount, 1);
      SWOSHook::ReadMemory(ptrJobOffers + m_DSeg, &m_JobOffers, sizeof(ASWSJobOffers));

      ImGui::Text("No of players: %d", m_PlayersCount);

      std::string mgrname = "Manager";
      //if (!std::string(m_Manager.firstname).empty())
      //  mgrname += std::string(m_Manager.firstname)+" ";
      //mgrname += std::string(m_Manager.surname);

      if (ImGui::CollapsingHeader(mgrname.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
      {
        ImGui::InputText("First Name", m_Manager.firstname, 9, ImGuiInputTextFlags_CharsUppercase);
        ImGui::InputText("Second Name", m_Manager.surname, 13, ImGuiInputTextFlags_CharsUppercase);
        const char* nationality_str[] = {
          "ALB", "AUT", "BEL", "BUL", "CRO", "CYP", "TCH", "DEN", "ENG", "EST", "FAR", "FIN",
          "FRA", "GER", "GRE", "HUN", "ICE", "ISR", "ITA", "LAT", "LIT", "LUX", "MLT", "HOL",
          "NIR", "NOR", "POL", "POR", "ROM", "RUS", "SMA", "SCO", "SLO", "SWE", "TUR", "UKR",
          "WAL", "SRB", "BLS", "SVK", "ESP", "ARM", "BOS", "AZB", "GEO", "SUI", "IRL", "MAC",
          "TKM", "LIE", "MOL", "CRC", "SAL", "GUA", "HON", "BHM", "MEX", "PAN", "USA", "BAH",
          "NIC", "BER", "JAM", "TRI", "CAN", "BAR", "ELS", "SVC", "ARG", "BOL", "BRA", "CHL",
          "COL", "ECU", "PAR", "SUR", "URU", "VEN", "GUY", "PER", "ALG", "SAF", "BOT", "BFS",
          "BUR", "LES", "ZAI", "ZAM", "GHA", "SEN", "CIV", "TUN", "MLI", "MDG", "CMR", "CHD",
          "UGA", "LIB", "MOZ", "KEN", "SUD", "SWA", "ANG", "TOG", "ZIM", "EGY", "TAN", "NIG",
          "ETH", "GAB", "SIE", "BEN", "CON", "GUI", "SRL", "MAR", "GAM", "MLW", "JAP", "TAI",
          "IND", "BAN", "BRU", "IRA", "JOR", "SRI", "SYR", "KOR", "IRN", "VIE", "MLY", "SAU",
          "YEM", "KUW", "LAO", "NKR", "OMA", "PAK", "PHI", "CHN", "SGP", "MAU", "MYA", "PAP",
          "TAD", "UZB", "QAT", "UAE", "AUS", "NZL", "FIJ", "SOL", "CUS"
        };
        int nation = m_Manager.nationality;
        ImGui::Combo("Nationality", &nation, nationality_str, 153);
        m_Manager.nationality = nation;
      }


      std::string namecarteam = "Career Team: "+std::string(m_MainTeam.name);
      if (ImGui::CollapsingHeader(namecarteam.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
      {
        ImGui::InputText("Team Name", m_MainTeam.name, 19, ImGuiInputTextFlags_CharsUppercase);
        int money = 0;
        SWOSHook::ReadMemory(m_DSeg + ptrNewBalance, &money, 4);
        ImGui::InputInt("Team Balance", &money, 0, 0);
        SWOSHook::WriteMemory(m_DSeg + ptrNewBalance, &money, 4);
        ImGui::Separator();
        if (ImGui::Button("Clear players injuries"))
          for (int i = 0; i < 26; i++)
            m_MainTeam.players[i].cardsinj.injury = 0;

        if (ImGui::Button("Clear players bans"))
          for (int i = 0; i < 26; i++)
            m_MainTeam.players[i].cardsinj.cards = 0;

        float wx = ImGui::GetContentRegionAvail().x;
        if (ImGui::BeginTable("Players", 11))
        {
          ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, wx * 0.35f);
          ImGui::TableSetupColumn("Pos",  ImGuiTableColumnFlags_WidthStretch, wx * 0.1f);
          ImGui::TableSetupColumn("P",    ImGuiTableColumnFlags_WidthStretch, wx * 0.05f);        
          ImGui::TableSetupColumn("V",    ImGuiTableColumnFlags_WidthStretch, wx * 0.05f);        
          ImGui::TableSetupColumn("H",    ImGuiTableColumnFlags_WidthStretch, wx * 0.05f);
          ImGui::TableSetupColumn("T",    ImGuiTableColumnFlags_WidthStretch, wx * 0.05f);        
          ImGui::TableSetupColumn("C",    ImGuiTableColumnFlags_WidthStretch, wx * 0.05f);        
          ImGui::TableSetupColumn("S",    ImGuiTableColumnFlags_WidthStretch, wx * 0.05f);        
          ImGui::TableSetupColumn("F",    ImGuiTableColumnFlags_WidthStretch, wx * 0.05f);      
          ImGui::TableSetupColumn("Val",  ImGuiTableColumnFlags_WidthStretch, wx * 0.1f);      
          ImGui::TableSetupColumn("Form",  ImGuiTableColumnFlags_WidthStretch, wx * 0.05f);      
          ImGui::TableHeadersRow();

          int pp = 0;
          for (int i = 0; i < 32; i++)
          {
            if (std::string(m_MainTeam.players[i].name).substr(0,5) != "*ERR*")
            {
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              const char* pos_str[] = {"GK", "RB", "LB", "D", "RW", "LW", "M", "A"};
              char pn[64] = {0};
              sprintf(pn, "###PName%02d", i);
              ImGui::InputText(pn, m_MainTeam.players[i].name, 23, ImGuiInputTextFlags_CharsUppercase);
              ImGui::TableNextColumn();

              sprintf(pn, "###PPosit%02d", i);
              int plposition = m_MainTeam.players[i].position.position;
              ImGui::Combo(pn, &plposition, pos_str, 8); //ImGui::SameLine();
              ImGui::TableNextColumn();
              m_MainTeam.players[i].position.position = plposition;

              sprintf(pn, "###PPass%02d", i);
              int att_p = m_MainTeam.players[i].attXP.second;
              ImGui::InputInt(pn, &att_p, 0, 0); 
              ImGui::TableNextColumn();
              m_MainTeam.players[i].attXP.second = att_p;

              sprintf(pn, "###PShots%02d", i);
              att_p = m_MainTeam.players[i].attVH.first;
              ImGui::InputInt(pn, &att_p, 0, 0); 
              ImGui::TableNextColumn();
              m_MainTeam.players[i].attVH.first = att_p;

              sprintf(pn, "###Phead%02d", i);
              att_p = m_MainTeam.players[i].attVH.second;
              ImGui::InputInt(pn, &att_p, 0, 0); 
              ImGui::TableNextColumn();
              m_MainTeam.players[i].attVH.second = att_p;

              sprintf(pn, "###Ptack%02d", i);
              att_p = m_MainTeam.players[i].attTC.first;
              ImGui::InputInt(pn, &att_p, 0, 0); 
              ImGui::TableNextColumn();
              m_MainTeam.players[i].attTC.first = att_p;

              sprintf(pn, "###Pbctrl%02d", i);
              att_p = m_MainTeam.players[i].attTC.second;
              ImGui::InputInt(pn, &att_p, 0, 0); 
              ImGui::TableNextColumn();
              m_MainTeam.players[i].attTC.second = att_p;
      
              sprintf(pn, "###Pspeed%02d", i);
              att_p = m_MainTeam.players[i].attSF.first;
              ImGui::InputInt(pn, &att_p, 0, 0); 
              ImGui::TableNextColumn();
              m_MainTeam.players[i].attSF.first = att_p;

              sprintf(pn, "###Pfini%02d", i);
              att_p = m_MainTeam.players[i].attSF.second;
              ImGui::InputInt(pn, &att_p, 0, 0); 
              ImGui::TableNextColumn();
              m_MainTeam.players[i].attSF.second = att_p;

              const char* swsval_str[] = {    
                "25K-", "25K", "30K", "40K", "50K", "65K", "75K", "85K", "100K", "110K", "130K",
                "150K", "160K", "180K", "200K", "250K", "300K", "350K", "450K", "500K", "550K",
                "600K", "650K", "700K", "750K", "800K", "850K", "950K", "1M", "1.1M", "1.3M",
                "1.5M", "1.6M", "1.8M", "1.9M", "2M", "2.25M", "2.75M", "3M", "3.5M", "4.5M",
                "5M", "6M", "7M", "8M", "9M", "10M", "12M", "15M", "15M+"
              };

              sprintf(pn, "###PValue%02d", i);
              int plvalue = m_MainTeam.players[i].value;
              ImGui::Combo(pn, &plvalue, swsval_str, 50); //ImGui::SameLine();
              m_MainTeam.players[i].value = plvalue;
              ImGui::TableNextColumn();

              sprintf(pn, "###Pform%02d", i);
              att_p = m_MainTeam.players[i].value_progress;
              ImGui::InputInt(pn, &att_p, 0, 0); 
              ImGui::TableNextColumn();
              m_MainTeam.players[i].value_progress = att_p;
              pp++;
            }
            if (pp == m_PlayersCount)
              break;
          }
        ImGui::EndTable();
        }
      }
        
      
      if (ImGui::CollapsingHeader("Job Offers", ImGuiTreeNodeFlags_DefaultOpen))
      {
        ImGui::Text("Number of Job Offers: %d", m_JobOffers.joboffers_count);
        for (int i = 0; i < m_JobOffers.joboffers_count; i++)
        {
          char tnme[64] = {0};
          sprintf(tnme, "TeamID##%d", i);
          ImGui::InputScalar(tnme, ImGuiDataType_U8, &m_JobOffers.joboffer[i].teamID.nation, 0, 0, "%x"); ImGui::SameLine();
          sprintf(tnme, "TeamNo##%d", i);
          ImGui::InputScalar(tnme, ImGuiDataType_U8, &m_JobOffers.joboffer[i].teamID.number);
          sprintf(tnme, "TeamName##%d", i);
          ImGui::InputText(tnme, m_JobOffers.joboffer[i].teamname, 19, ImGuiInputTextFlags_CharsUppercase);
          sprintf(tnme, "Budget##%d", i);
          ImGui::InputScalar(tnme, ImGuiDataType_S32, &m_JobOffers.joboffer[i].budget);
          ImGui::Separator();

        }
      }

      SWOSHook::WriteMemory(m_DSeg + ptrCareerTeam, &m_MainTeam, sizeof(ASWSTeamCar));
      SWOSHook::WriteMemory(m_DSeg + ptrCareerManager, &m_Manager, sizeof(ASWSCarManager));
      SWOSHook::WriteMemory(ptrJobOffers + m_DSeg, &m_JobOffers, sizeof(ASWSJobOffers));
      
    }
    if (ImGui::CollapsingHeader("About", ImGuiTreeNodeFlags_DefaultOpen))
    {
      ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Anx-SWOS-CarRTE 0.1.3b");
      ImGui::Text("Copyright (c)2024 AnoXic");
      ImGui::Separator();
      ImGui::Text("Enable/Disable: LCtrl + e");
    }
    ImGui::End();

  }

  // RENDER
  ImGui::Render();
  if (m_OpenGLRenderer)
  {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
      SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
      SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }
    SDL_GL_SwapWindow(m_Window);
  }
  else
  {
    SDL_RenderSetScale(m_Renderer, 1, 1);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(m_Renderer);
  }
}