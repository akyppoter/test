#pragma once
#include "rage/classes.h"
#include "manager/manager.h"

namespace commands::features
{
	inline CPed* cPed{};
	inline CPedWeaponManager* cPedWeaponManager{};
	inline CWeaponInfo* cWeaponInfo{};
	inline CAmmoInfo* cAmmoInfo{};
	inline CVehicle* cVehicle{};
	inline CPlayerInfo* cPlayerInfo{};

	namespace self
	{
		inline bool norb = false;
		inline bool aimBot = false;
		inline bool esppp = false;
		inline bool grav = false;
		inline bool DropRP = false;
		inline bool bnker = false;
		inline bool Droprp = false;
		inline bool DropM = false;
		inline bool Rman = false;
		inline bool Nclub = false;
		inline bool nclip = false;
		inline bool dphone = false;
		inline bool fcam = false;
		inline bool mradio = false;
		inline bool taPlayer = false;
		inline bool PedFly = false;
		inline bool winvis = false;
		inline bool pInvis = false;
		inline bool offradar = false;
		inline bool tPlayer = false;
		inline bool HHud = false;
		inline bool Eammo = false;
		inline bool HBoost = false;
		inline bool Sswim = false;
		inline bool RFire = false;
		inline bool damagemulti = false;
		inline bool invisa = false;
		inline bool ghostmode = false;
		inline bool vgmode = false;
		inline bool awanted = false;
		inline bool Nwater = false;
		inline bool nRagdoll = false;
		inline bool ivisable = false;
		inline bool Unlimox = false;
		inline bool superrun = false;
		inline bool neverwanted = false;
		inline bool Unlimitedammo = false;
		inline bool slowmo = false;
		inline bool disable_godmode = false;
		inline bool sUperman = false;
		
		extern void god_mode(toggleCommand* command);
		extern void beast_jump(toggleCommand* command);
		extern void graceful_landing(toggleCommand* command);
	}

	extern void init();
	extern void uninit();
	extern void on_init();
	extern void on_tick();
}
