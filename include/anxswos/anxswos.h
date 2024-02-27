#pragma once
#include <SDL2/SDL.h>
#include <gl/glew.h>
#include <SDL2/SDL_opengl.h>
#include "swoshook.h"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdlrenderer2.h"
#include "imgui_memory_editor.h"
#include "anxtex.h"

#pragma pack(push, 1)

struct ASWSAttributte
{
  uint8_t second: 4;
  uint8_t first: 4;
};

struct ASWSMultiPos
{
  uint8_t unused : 3;
  uint8_t skin : 2;
  uint8_t position : 3;
};

struct ASWSInjCard
{
  uint8_t cards : 4;
  uint8_t injury : 4;
};

struct ASWSKit
{
  uint8_t type;         /** type of kit: 0 - plain, 1 - slaves, 2 - v-lines 3 - h-lines */
  uint8_t shirt_primary_col;   
  uint8_t shirt_secondary_col;
  uint8_t short_col;
  uint8_t socks_col;
};

struct ASWSPlayer
{
  uint8_t nationality;
  uint8_t res0;
  uint8_t number;
  char name[23];
  ASWSMultiPos position;
  ASWSInjCard cardsinj;
  ASWSAttributte attXP;
  ASWSAttributte attVH;
  ASWSAttributte attTC;
  ASWSAttributte attSF;
  uint8_t value;
  uint8_t lgegoals;
  uint8_t cupgoals;
  uint8_t unkgoals;
  uint8_t eurgoals;
  int8_t value_progress;  /** add/sub market value from original **/
};

struct ASWSTeamID
{
  uint8_t nation;
  uint8_t number;
};

struct ASWSTeam
{
  ASWSTeamID teamid;
  uint16_t swsgenno;          /** big-endian SWS_Gen_No **/
  uint8_t res0;
  char name[19];
  uint8_t tactic;
  uint8_t division;
  ASWSKit homekit;
  ASWSKit awaykit;
  char coachname[24];
  uint8_t playerpos[16];
  ASWSPlayer players[16];
};

struct ASWSTeamCar
{
  ASWSTeamID teamid;
  uint16_t swsgenno;          /** big-endian SWS_Gen_No **/
  uint8_t res0;
  char name[19];
  uint8_t tactic;
  uint8_t division;
  ASWSKit homekit;
  ASWSKit awaykit;
  char coachname[24];
  uint8_t playerpos[16];
  ASWSPlayer players[32];
};

struct ASWSTransfer
{
  ASWSTeamID teamfrom_id;
  uint8_t playerpos;
  uint8_t unknown;
  ASWSPlayer player;
};

struct ASWSTeamCarName
{
  uint16_t players_no;
  int16_t unk_0;
  int16_t unk_1;
  ASWSTeamID teamid;
  char name[19];
};

struct ASWSCarManager
{
  ASWSTeamID current_team;
  int16_t player_coach;
  int16_t teamleague;
  int16_t unknown[4];
  int16_t mrs_ms;
  char mrs_ms_str[5];
  char firstname[9];
  char surname[13];
  int8_t nationality;
};

struct ASWSJobOffer
{
  ASWSTeamID teamID;
  char teamname[18];
  int32_t budget;
  int16_t unknown;
};

struct ASWSJobOffers
{
  int16_t joboffers_count;
  ASWSJobOffer joboffer[5];
};

#pragma pack(pop)


class AnxSWOS
{
private:
  uintptr_t m_Base;
  uintptr_t m_DSeg;
  SDL_Window* m_Window;
  SDL_GLContext m_GLContext;
  SDL_Renderer* m_Renderer;
  ImGuiContext* m_ImGuiCtx;
  Texture* m_Background;
  SDL_Texture* m_BackgroundSDL;
  bool m_GUIOverlay;
  bool m_OpenGLRenderer;
  bool m_GUIEnabled;
  bool IsCareer();
  //
  ASWSTeamCar m_MainTeam;
  ASWSCarManager m_Manager;
  uint8_t m_PlayersCount;
  ASWSJobOffers m_JobOffers;
  //
  bool m_HexMemory;
  MemoryEditor m_HexEdit;
  uintptr_t m_CurrentHexAddress;
  size_t m_DataSize;
  SDL_Texture* loadSDLTexture(const char* path);
public:
  AnxSWOS(uintptr_t base, bool overlay);
  virtual ~AnxSWOS();
  void Init();
  void OnEvent(SDL_Event* e);
  void Draw();
};