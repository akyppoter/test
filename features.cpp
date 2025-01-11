#include "features.h"
#include "fiber/pool.h"
#include "hooking/hooking.h"
#include "rage/lists.h"
#include "renderer/renderer.h"
inline bool orbcooldown{};
static const int GLOBAL_OFFRADAR[] = { 2657589, 466, 210 };
static const int GLOBAL_NETWORK_STOPWATCH[] = { 2672505, 56 };


#define M_PI 3.14159265358979323846

namespace self2
{
	static const ControllerInputs controls[] = { ControllerInputs::INPUT_LOOK_LR, ControllerInputs::INPUT_LOOK_UD, ControllerInputs::INPUT_LOOK_UP_ONLY, ControllerInputs::INPUT_LOOK_DOWN_ONLY, ControllerInputs::INPUT_LOOK_LEFT_ONLY, ControllerInputs::INPUT_LOOK_RIGHT_ONLY, ControllerInputs::INPUT_LOOK_LEFT, ControllerInputs::INPUT_LOOK_RIGHT, ControllerInputs::INPUT_LOOK_UP, ControllerInputs::INPUT_LOOK_DOWN };
	template<typename T>
	union vector2
	{
		T data[2];
		struct { T x, y; };

		vector2(T x, T y) :
			x(x),
			y(y)
		{
		}

		vector2() :
			x(),
			y()
		{
		}
	};

	template<typename T>
	union vector3
	{
		T data[3];
		struct { T x, y, z; };

		vector3(T x, T y, T z) :
			x(x),
			y(y),
			z(z)
		{
		}

		vector3() :
			x(),
			y(),
			z()
		{
		}
	};

	template<typename T>
	union vector4
	{
		T data[4];
		struct { T x, y, z, w; };

		vector4(T x, T y, T z, T w) :
			x(x),
			y(y),
			z(z),
			w(w)
		{
		}

		vector4() :
			x(),
			y(),
			z(),
			w()
		{
		}
	};

	template<typename T>
	union matrix34
	{
		T data[3][4];
		struct { struct { T x, y, z, w; } rows[3]; };
	};

	template<typename T>
	union matrix44
	{
		T data[4][4];
		struct { struct { T x, y, z, w; } rows[4]; };
	};
	using get_gameplay_cam_coords = Vector3(*)();
	get_gameplay_cam_coords m_get_gameplay_cam_coords;
	typedef vector2<float> fvector2;
	typedef vector3<float> fvector3;
	typedef vector4<float> fvector4;
	typedef matrix34<float> fmatrix34;
	typedef matrix44<float> fmatrix44;
}

namespace big::math
{
	inline float deg_to_rad(float deg)
	{
		double radian = (3.14159265359 / 180) * deg;
		return (float)radian;
	}

	inline float distance_between_vectors(Vector3 a, Vector3 b)
	{
		return (float)sqrt(pow((a.x - b.x), 2) + pow((a.y - b.y), 2) + pow((a.z - b.z), 2));
	}

	inline float distance_between_vectors(self2::fvector3 a, self2::fvector3 b)
	{
		return (float)sqrt(pow((a.x - b.x), 2) + pow((a.y - b.y), 2) + pow((a.z - b.z), 2));
	}

	inline Vector3 rotation_to_direction(Vector3 rotation)
	{
		float x = deg_to_rad(rotation.x);
		float z = deg_to_rad(rotation.z);

		float num = abs(cos(x));

		return Vector3{ -sin(z) * num, cos(z) * num, sin(x) };
	}

	inline float calculate_distance_from_game_cam(rage::vector3 player_position)
	{

		const Vector3 plyr_coords = { player_position.x, player_position.y, player_position.z };
		const Vector3 cam_coords = self2::m_get_gameplay_cam_coords();

		return (float)distance_between_vectors(plyr_coords, cam_coords);
	}

	inline Vector3 raycast_coords(Vector3 coord, Vector3 rot, Entity ignore)
	{
		BOOL hit;
		Vector3 end_coords;
		Vector3 surface_normal;
		Entity hit_entity;
		Vector3 dir = math::rotation_to_direction(rot);
		Vector3 far_coords;

		far_coords.x = coord.x + dir.x * 1000;
		far_coords.y = coord.y + dir.y * 1000;
		far_coords.z = coord.z + dir.z * 1000;

		int ray = SHAPETEST::START_EXPENSIVE_SYNCHRONOUS_SHAPE_TEST_LOS_PROBE(coord.x,
			coord.y,
			coord.z,
			far_coords.x,
			far_coords.y,
			far_coords.z,
			-1,
			ignore,
			7);
		SHAPETEST::GET_SHAPE_TEST_RESULT(ray, &hit, &end_coords, &surface_normal, &hit_entity);
		return end_coords;
	}
}

namespace Big
{
	class GameVariables
	{
	public:
		explicit GameVariables();
		~GameVariables() noexcept = default;
		GameVariables(GameVariables const&) = delete;
		GameVariables(GameVariables&&) = delete;
		GameVariables& operator=(GameVariables const&) = delete;
		GameVariables& operator=(GameVariables&&) = delete;
		bool* is_session_started{};
		void PostInit();

		std::uint32_t* m_GameState;
		std::uint32_t* m_FrameCount;
		PVOID ModelBypass;
		HWND m_GameWindow;
		IDXGISwapChain* m_Swapchain;
		rage::scrNativeRegistration** m_NativeRegistrations;
		std::uint64_t** m_GlobalBase;
		char* m_GameBuild;
		__int64** m_globalPtr;
	};

	class GameFunctions
	{
	public:
		explicit GameFunctions();
		~GameFunctions() noexcept = default;
		GameFunctions(GameFunctions const&) = delete;
		GameFunctions(GameFunctions&&) = delete;
		GameFunctions& operator=(GameFunctions const&) = delete;
		GameFunctions& operator=(GameFunctions&&) = delete;

		using IsDlcPresent = bool(std::uint32_t hash);
		IsDlcPresent* m_IsDlcPresent;

		using WndProc = LRESULT(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		WndProc* m_WndProc;

		using GetEventData = bool(std::int32_t eventGroup, std::int32_t eventIndex, std::int64_t* args, std::uint32_t argCount);
		GetEventData* m_GetEventData;

		using GetLabelText = const char* (void* unk, const char* label);
		GetLabelText* m_GetLabelText;
	};

	inline std::unique_ptr<GameVariables> g_GameVariables;
	inline std::unique_ptr<GameFunctions> g_GameFunctions;
}
class globalHandle
{
private:
	void* _handle;

public:
	globalHandle(int index)
		: _handle(&Big::g_GameVariables->m_globalPtr[index >> 18 & 0x3F][index & 0x3FFFF])
	{ }

	globalHandle(void* p)
		: _handle(p)
	{ }

	globalHandle(const globalHandle& copy)
		: _handle(copy._handle)
	{ }

	globalHandle At(int index)
	{
		return globalHandle(reinterpret_cast<void**>(this->_handle) + (index));
	}

	globalHandle At(int index, int size)
	{
		// Position 0 = Array Size
		return this->At(1 + (index * size));
	}

	template <typename T>
	T* Get()
	{
		return reinterpret_cast<T*>(this->_handle);
	}

	template <typename T>
	T& As()
	{
		return *this->Get<T>();
	}
};




namespace commands::features
{
	namespace self
	{
		u32 m_last_god_mode_bits = 0;
		u32 god_mode_bits = 0;
		inline Ped ped;
		inline Player id;
		inline Vector3 pos;
		inline Vector3 rot;
		inline Vehicle veh;
		inline int char_index;

		void god_mode(toggleCommand* command)
		{
			if (cPed == nullptr)
			{
				LOG(Warn, "Cped is nullptr")
					return;
			}
			if ("godMode"_TC->get(0).toggle && !disable_godmode)
			{
				god_mode_bits |= 0x100;
				// Calculate changed bits
				const u32 changed_bits = god_mode_bits ^ m_last_god_mode_bits;
				// Calculate combined bits (enabled bits or changed bits)
				const u32 enabled_or_changed_bits = god_mode_bits | changed_bits;
				// Calculate unchanged bits by clearing changed or enabled bits
				const u32 unchanged_bits = cPed->m_damage_bits & ~enabled_or_changed_bits;
				// Set the character's damage bits with god mode bits
				cPed->m_damage_bits = unchanged_bits | god_mode_bits;
				disable_godmode = true;
			}
			if (cPed != nullptr && !"godMode"_TC->get(0).toggle && disable_godmode)
			{
				cPed->m_damage_bits = NULL;
				disable_godmode = false;
			}
		}
		
		//void gravity(toggleCommand* command)
		//{
		//	if ("gravityy"_TC->get(0).toggle && !grav)
		//	{
		//		MISC::SET_GRAVITY_LEVEL(0);
		//		grav = true;
		//	}
		//	if (cPed != nullptr && !"gravityy"_TC->get(0).toggle && grav)
		//	{
		//		
		//		grav = false;
		//	}
		//}
		void beast_jump(toggleCommand* command)
		{
			
		}
		void Uammo(toggleCommand* command)
		{

			if ("uammo"_TC->get(0).toggle && !Unlimitedammo)
			{
				WEAPON::SET_PED_INFINITE_AMMO_CLIP(PLAYER::PLAYER_PED_ID(), true);
				Unlimitedammo = true;
			}
			if (cPed != nullptr && !"uammo"_TC->get(0).toggle && Unlimitedammo)
			{
				WEAPON::SET_PED_INFINITE_AMMO_CLIP(PLAYER::PLAYER_PED_ID(), false);
				Unlimitedammo = false;
			}
		}

		void SlowMode(toggleCommand* command)
		{
			if ("slowmode"_TC->get(0).toggle && !slowmo)
			{
				MISC::SET_TIME_SCALE(0.2);
				slowmo = true;
			}
			if (cPed != nullptr && !"slowmode"_TC->get(0).toggle && slowmo)
			{
				MISC::SET_TIME_SCALE(1);
				slowmo = false;
			}



		}

		


		void neverWanted(toggleCommand* command)
		{
			if ("nWanted"_TC->get(0).toggle && !neverwanted)
			{
				using Ped = Entity;
				using Object = Entity;
				Entity player = PLAYER::PLAYER_PED_ID();
				PLAYER::CLEAR_PLAYER_WANTED_LEVEL(PLAYER::PLAYER_ID());
				PLAYER::SET_MAX_WANTED_LEVEL(0);
				PLAYER::SET_POLICE_IGNORE_PLAYER(player, true);
				PLAYER::SET_PLAYER_CAN_BE_HASSLED_BY_GANGS(player, false);
				PLAYER::SET_EVERYONE_IGNORE_PLAYER(player, true);
				PLAYER::SET_IGNORE_LOW_PRIORITY_SHOCKING_EVENTS(player, true);
				neverwanted = true;
			}
			if (cPed != nullptr && !"nWanted"_TC->get(0).toggle && neverwanted)
			{
				Entity player = PLAYER::PLAYER_PED_ID();
				PLAYER::CLEAR_PLAYER_WANTED_LEVEL(PLAYER::PLAYER_ID());
				PLAYER::SET_MAX_WANTED_LEVEL(5);
				PLAYER::SET_POLICE_IGNORE_PLAYER(player, false);
				PLAYER::SET_PLAYER_CAN_BE_HASSLED_BY_GANGS(player, false);
				PLAYER::SET_EVERYONE_IGNORE_PLAYER(player, false);
				PLAYER::SET_IGNORE_LOW_PRIORITY_SHOCKING_EVENTS(player, false);
				neverwanted = false;
			}

		}

		void Ghost(toggleCommand* command)
		{
			if ("ghmode"_TC->get(0).toggle && !ghostmode)
			{

				NETWORK::SET_LOCAL_PLAYER_AS_GHOST(true, true);




				ghostmode = true;
			}
			if (cPed != nullptr && !"ghmode"_TC->get(0).toggle && ghostmode)
			{

				NETWORK::SET_LOCAL_PLAYER_AS_GHOST(false, false);

				ghostmode = false;
			}

		}


		void alwaysWanted(toggleCommand* command)
		{
			if ("aWanted"_TC->get(0).toggle && !awanted)
			{
				using Ped = Entity;
				using Object = Entity;
				Entity player = PLAYER::PLAYER_PED_ID();
				PLAYER::SET_MAX_WANTED_LEVEL(5);
				cPed->m_player_info->m_wanted_level = 5;
				PLAYER::SET_POLICE_IGNORE_PLAYER(player, false);
				PLAYER::SET_PLAYER_CAN_BE_HASSLED_BY_GANGS(player, false);
				PLAYER::SET_EVERYONE_IGNORE_PLAYER(player, false);
				PLAYER::SET_IGNORE_LOW_PRIORITY_SHOCKING_EVENTS(player, false);

				awanted = true;
			}
			if (cPed != nullptr && !"aWanted"_TC->get(0).toggle && awanted)
			{
				Entity player = PLAYER::PLAYER_PED_ID();
				PLAYER::CLEAR_PLAYER_WANTED_LEVEL(PLAYER::PLAYER_ID());
				PLAYER::SET_MAX_WANTED_LEVEL(0);
				PLAYER::SET_POLICE_IGNORE_PLAYER(player, true);
				PLAYER::SET_PLAYER_CAN_BE_HASSLED_BY_GANGS(player, false);
				PLAYER::SET_EVERYONE_IGNORE_PLAYER(player, true);
				PLAYER::SET_IGNORE_LOW_PRIORITY_SHOCKING_EVENTS(player, true);
				awanted = false;
			}

		}
		void SuperRun(toggleCommand* command)
		{
			if ("srun"_TC->get(0).toggle && !superrun)
			{
				cPed->m_player_info->m_run_speed = 5.f;
				superrun = true;
			}
			if (cPed != nullptr && !"srun"_TC->get(0).toggle && superrun)
			{
				cPed->m_player_info->m_run_speed = 1.f;
				superrun = false;
			}

		}
		void Superswim(toggleCommand* command)
		{
			if ("sswim"_TC->get(0).toggle && !Sswim)
			{
				cPed->m_player_info->m_swim_speed = 5.f;
				Sswim = true;
			}
			if (cPed != nullptr && !"sswim"_TC->get(0).toggle && Sswim)
			{
				cPed->m_player_info->m_swim_speed = 1.f;
				Sswim = false;
			}

		}
		void UnlimitedOxegen(toggleCommand* command)
		{
			if ("uox"_TC->get(0).toggle && !Unlimox)
			{
				PED::SET_PED_CONFIG_FLAG(PLAYER::PLAYER_PED_ID(), 3, false);


				Unlimox = true;
			}
			if (cPed != nullptr && !"uox"_TC->get(0).toggle && Unlimox)
			{
				PED::SET_PED_CONFIG_FLAG(PLAYER::PLAYER_PED_ID(), 3, true);
				Unlimox = false;

			}

		}
		void Nowcollision(toggleCommand* command)
		{
			if ("nwcollision"_TC->get(0).toggle && !Nwater)
			{

				cPed->m_navigation->m_damp->m_water_collision = 0, !Nwater;
				cVehicle->m_navigation->m_damp->m_water_collision = 0, !Nwater;
				Nwater = true;

			}
			if (cPed != nullptr && !"nwcollision"_TC->get(0).toggle && Nwater)
			{
				cPed->m_navigation->m_damp->m_water_collision = 1, !Nwater;
				cVehicle->m_navigation->m_damp->m_water_collision = 0, !Nwater;
				Nwater = false;


			}

		}
		
		void Noragdoll(toggleCommand* command)
		{
			if ("ndoll"_TC->get(0).toggle && !nRagdoll)
			{
			
				PED::SET_PED_CAN_RAGDOLL(PLAYER::PLAYER_PED_ID(), false);
				PED::SET_PED_CONFIG_FLAG(PLAYER::PLAYER_PED_ID(), 32, false);
				PED::SET_PED_CAN_BE_KNOCKED_OFF_VEHICLE(PLAYER::PLAYER_PED_ID(), 1);

				nRagdoll = true;

			}
			else if (cPed != nullptr && !"ndoll"_TC->get(0).toggle && nRagdoll)
			{


				PED::SET_PED_CAN_RAGDOLL(PLAYER::PLAYER_PED_ID(), true);
				PED::SET_PED_CONFIG_FLAG(PLAYER::PLAYER_PED_ID(), 32, true);
				PED::SET_PED_CAN_BE_KNOCKED_OFF_VEHICLE(PLAYER::PLAYER_PED_ID(), 1);




				nRagdoll = false;

			}

		}

		void Tplayer(toggleCommand* command)
		{
			if ("tinyp"_TC->get(0).toggle && !tPlayer)
			{
				PED::SET_PED_CONFIG_FLAG(PLAYER::PLAYER_PED_ID(), 223, !tPlayer);
				tPlayer = true;

			}
			if (cPed != nullptr && !"tinyp"_TC->get(0).toggle && tPlayer)
			{

				PED::SET_PED_CONFIG_FLAG(PLAYER::PLAYER_PED_ID(), 223, !tPlayer);

				tPlayer = false;

			}

		}
		void esp(toggleCommand* command)
		{
			if ("esp"_TC->get(0).toggle && !esppp)
			{


#define PlayersSessionForLoop for(int i = 0; i < 32; i++)
				PlayersSessionForLoop
					if (PLAYER::PLAYER_ID() != i)
					{
						Vector3 entitylocation = ENTITY::GET_ENTITY_COORDS(PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i), false);
						Vector3 top1world = { entitylocation.x + 0.3f, entitylocation.y + 0.3f, entitylocation.z + 0.9f };
						Vector3 top2world = { entitylocation.x - 0.3f, entitylocation.y + 0.3f, entitylocation.z + 0.9f };
						Vector3 top3world = { entitylocation.x + 0.3f, entitylocation.y - 0.3f, entitylocation.z + 0.9f };
						Vector3 top4world = { entitylocation.x - 0.3f, entitylocation.y - 0.3f, entitylocation.z + 0.9f };
						Vector3 bottom1world = { entitylocation.x + 0.3f, entitylocation.y + 0.3f, entitylocation.z - 0.9f };
						Vector3 bottom2world = { entitylocation.x - 0.3f, entitylocation.y + 0.3f, entitylocation.z - 0.9f };
						Vector3 bottom3world = { entitylocation.x + 0.3f, entitylocation.y - 0.3f, entitylocation.z - 0.9f };
						Vector3 bottom4world = { entitylocation.x - 0.3f, entitylocation.y - 0.3f, entitylocation.z - 0.9f };
						GRAPHICS::DRAW_LINE(bottom1world.x, bottom1world.y, bottom1world.z, bottom2world.x, bottom2world.y, bottom2world.z, 255, 255, 255, 255); // Bottom line
						GRAPHICS::DRAW_LINE(bottom2world.x, bottom2world.y, bottom2world.z, top2world.x, top2world.y, top2world.z, 255, 255, 255, 255);        // Right side line
						GRAPHICS::DRAW_LINE(top2world.x, top2world.y, top2world.z, top1world.x, top1world.y, top1world.z, 255, 255, 255, 255);                // Top line
						GRAPHICS::DRAW_LINE(top1world.x, top1world.y, top1world.z, bottom1world.x, bottom1world.y, bottom1world.z, 255, 255, 255, 255);      // Left side line



						Vector3 locationOne = ENTITY::GET_ENTITY_COORDS(PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i), false);
						Vector3 locationTwo = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), false);
						//GRAPHICS::DRAW_LINE(locationOne.x, locationOne.y, locationOne.z, locationTwo.x, locationTwo.y, locationTwo.z,255, 255, 255, 255);
						GRAPHICS::DRAW_MARKER(42, locationOne.x, locationOne.y, locationOne.z + 1.25, 0, 0, 0, 0, 180, 0, 0.25, 0.25, 0.25, 255, 0, 0, 255, 1, 1, 1, 0, 0, 0, 0);

					}
				if (cPed != nullptr && !"esp"_TC->get(0).toggle && esppp)
				{


					esppp = false;

				}

			}
		}







		void VehGodmode(toggleCommand* command)
		{
			if ("vGmode"_TC->get(0).toggle && !vgmode)
			{
				const auto vehicle = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), false);
				if (PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), false))
				{
					ENTITY::SET_ENTITY_INVINCIBLE(vehicle, true);
					VEHICLE::SET_VEHICLE_DAMAGE(vehicle, 0.f, 0.f, 0.f, 0.f, 200.f, false);
					VEHICLE::SET_VEHICLE_FIXED(vehicle);
					VEHICLE::SET_VEHICLE_EXPLODES_ON_HIGH_EXPLOSION_DAMAGE(vehicle, false);
					VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(vehicle, false);
					VEHICLE::SET_VEHICLE_WHEELS_CAN_BREAK(vehicle, false);
					VEHICLE::SET_VEHICLE_CAN_BE_VISIBLY_DAMAGED(vehicle, false);
				}
				vgmode = true;
			}
			if (cPed != nullptr && !"vGmode"_TC->get(0).toggle && vgmode)
			{
				const auto vehicle = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), false);
				if (PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), false))
				{
					ENTITY::SET_ENTITY_INVINCIBLE(vehicle, false);
					VEHICLE::SET_VEHICLE_DAMAGE(vehicle, 0.f, 0.f, 0.f, 0.f, 200.f, false);
					VEHICLE::SET_VEHICLE_FIXED(vehicle);
					VEHICLE::SET_VEHICLE_EXPLODES_ON_HIGH_EXPLOSION_DAMAGE(vehicle, true);
					VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(vehicle, true);
					VEHICLE::SET_VEHICLE_WHEELS_CAN_BREAK(vehicle, true);
					VEHICLE::SET_VEHICLE_CAN_BE_VISIBLY_DAMAGED(vehicle, true);
				}
				vgmode = false;
			}

		}





		void dMg(toggleCommand* command)
		{
			if ("dMulti"_TC->get(0).toggle && !damagemulti)
			{
				cPed->m_weapon_manager->m_weapon_info->m_damage *= 1000;

				damagemulti = true;

			}
			if (cPed != nullptr && !"dMulti"_TC->get(0).toggle && damagemulti)
			{
				cPed->m_weapon_manager->m_weapon_info->m_damage /= 1000;
				damagemulti = false;

			}


		}
		enum WeaponHash : DWORD
		{
			WEAPON_INVALID = 0xFFFFFFFF,
			WEAPON_PISTOL = 0x1B06D571,
			WEAPON_RPG = 0xB1CA77B1,
			WEAPON_FIREWORK = 0x7F7497E5,
			// Add more weapon hashes as needed
		};
		enum EXPLOSION : DWORD
		{
			ROCKET = 0x2F9D95AA,

			// Add more weapon hashes as needed
		};

		void ExploAmmo(toggleCommand* command) {
			// Check if the toggle is on
			if ("eammo"_TC->get(0).toggle) {
				// Fire a rocket if the player is shooting
				if (PAD::IS_CONTROL_PRESSED(0, 24)) {
					Vector3 playerPosition = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true);

					// Get the player's camera direction
					Vector3 cameraRotation = CAM::GET_GAMEPLAY_CAM_ROT(0);
					float cameraYaw = cameraRotation.z;
					float cameraPitch = cameraRotation.x;

					// Calculate direction vector from camera angles
					Vector3 playerDirection;
					playerDirection.x = -sin(cameraYaw * (M_PI / 180.0f));
					playerDirection.y = cos(cameraYaw * (M_PI / 180.0f));
					playerDirection.z = tan(cameraPitch * (M_PI / 180.0f));

					// Normalize the direction vector
					float length = sqrt(playerDirection.x * playerDirection.x + playerDirection.y * playerDirection.y + playerDirection.z * playerDirection.z);
					playerDirection.x /= length;
					playerDirection.y /= length;
					playerDirection.z /= length;

					// Calculate the endpoint of the rocket's trajectory
					Vector3 endPosition;
					endPosition.x = playerPosition.x + playerDirection.x * 20.0f;
					endPosition.y = playerPosition.y + playerDirection.y * 20.0f;
					endPosition.z = playerPosition.z + playerDirection.z * 20.0f;

					// Define the explosion hash
					Hash explosionHash = 59; // ROCKET

					// Create explosion at the endpoint
					FIRE::ADD_EXPLOSION(endPosition.x, endPosition.y, endPosition.z, explosionHash, 10.0f, true, false, 0.0f, false);



				}
			}
		}

		void Rocketman(toggleCommand* command)
		{
			if ("rman"_TC->get(0).toggle && !Rman) {
				if (WEAPON::HAS_PED_GOT_WEAPON(PLAYER::PLAYER_PED_ID(), WeaponHash::WEAPON_PISTOL, false))
				{
					Rman = true;
				}
				else
				{
					Rman = false;
				}
			}

			if (Rman)
			{
				Vector3 playerPosition = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true);
				Vector3 playerDirection = ENTITY::GET_ENTITY_FORWARD_VECTOR(PLAYER::PLAYER_PED_ID());
				Vector3 endPosition = playerPosition + playerDirection * 100.0f; // Adjust the range as needed

				// Shoot RPG bullet
				MISC::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(
					playerPosition.x, playerPosition.y, playerPosition.z,
					endPosition.x, endPosition.y, endPosition.z,
					100, // Damage
					true, WeaponHash::WEAPON_FIREWORK, PLAYER::PLAYER_PED_ID(), true, false, -1.0f
				);
			}

			if (cPed != nullptr && !"rman"_TC->get(0).toggle && Rman)
			{
				MISC::GET_HASH_KEY("WEAPON_FIREWORK"), PLAYER::PLAYER_PED_ID(), false, false, -1.0f;
				Rman = false;
			}
		}





		void graceful_landing(toggleCommand* command)
		{

		}
		void pEdfly(toggleCommand* command)
		{
			if ("pedfly"_TC->get(0).toggle && !PedFly)
			{
				cPed->m_weapon_manager->m_weapon_info->m_force_on_ped *= 100;



				PedFly = true;

			}
			if (cPed != nullptr && !"pedfly"_TC->get(0).toggle && PedFly)
			{
				cPed->m_weapon_manager->m_weapon_info->m_force_on_ped /= 100;
				PedFly = false;

			}
		}
		Cam camera = -1;
		Vector3 position;
		Vector3 rotation;
		Vector3 vecChange = { 0.f, 0.f, 0.f };
		float speed = 0.5f;
		float mult = 0.f;
		bool can_update_location()
		{
			return fcam && CAM::DOES_CAM_EXIST(camera);
		}
		static void Fcam(toggleCommand* command)
		{
			if ("fCam"_TC->get(0).toggle && !fcam)
			{
				camera = CAM::CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", 0);

				if (CAM::DOES_CAM_EXIST(camera))
				{
					position = CAM::GET_GAMEPLAY_CAM_COORD();
					rotation = CAM::GET_GAMEPLAY_CAM_ROT(2);

					ENTITY::FREEZE_ENTITY_POSITION(self::veh, true);

					CAM::SET_CAM_COORD(camera, position.x, position.y, position.z);
					CAM::SET_CAM_ROT(camera, rotation.x, rotation.y, rotation.z, 2);
					CAM::SET_CAM_ACTIVE(camera, true);
					CAM::RENDER_SCRIPT_CAMS(true, true, 500, true, true, 0);

					fcam = true;
				}
				else
				{
					// Handle error: camera creation failed
					fcam = false; // Ensure fcam is set to false if camera creation fails
				}
			}
			else if (fcam)
			{
				PAD::DISABLE_ALL_CONTROL_ACTIONS(0);

				for (const auto& control : self2::controls)
					PAD::ENABLE_CONTROL_ACTION(0, static_cast<int>(control), true);

				Vector3 vecChange = { 0.f, 0.f, 0.f };

				if (can_update_location())
				{
					// Calculate camera pitch angle
					Vector3 rot = CAM::GET_CAM_ROT(camera, 2);
					float pitch = big::math::deg_to_rad(rot.x);

					// Left Shift
					if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)ControllerInputs::INPUT_SPRINT))
						vecChange.z += speed / 2;
					// Left Control
					if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)ControllerInputs::INPUT_DUCK))
						vecChange.z -= speed / 2;
					// Forward
					if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)ControllerInputs::INPUT_MOVE_UP_ONLY))
						vecChange.y -= speed * sin(pitch); // Move down
					// Backward
					if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)ControllerInputs::INPUT_MOVE_DOWN_ONLY))
						vecChange.y += speed * sin(pitch); // Move up
					// Left
					if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)ControllerInputs::INPUT_MOVE_LEFT_ONLY))
						vecChange.x -= speed;
					// Right
					if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)ControllerInputs::INPUT_MOVE_RIGHT_ONLY))
						vecChange.x += speed;
				}

				if (vecChange.x == 0.f && vecChange.y == 0.f && vecChange.z == 0.f)
					mult = 0.f;
				else if (mult < 10)
					mult += 0.15f;

				Vector3 rot = CAM::GET_CAM_ROT(camera, 2);
				float yaw = big::math::deg_to_rad(rot.z); // horizontal

				position.x += (vecChange.x * cos(yaw) - vecChange.y * sin(yaw)) * mult;
				position.y += (vecChange.x * sin(yaw) + vecChange.y * cos(yaw)) * mult;
				position.z += vecChange.z * mult;

				CAM::SET_CAM_COORD(camera, position.x, position.y, position.z);
				STREAMING::SET_FOCUS_POS_AND_VEL(position.x, position.y, position.z, 0.f, 0.f, 0.f);

				rotation = CAM::GET_GAMEPLAY_CAM_ROT(2);
				CAM::SET_CAM_ROT(camera, rotation.x, rotation.y, rotation.z, 2);
			}
			if (!("fCam"_TC->get(0).toggle) && fcam)
			{
				CAM::SET_CAM_ACTIVE(camera, false);
				CAM::RENDER_SCRIPT_CAMS(false, true, 500, true, true, 0);
				CAM::DESTROY_CAM(camera, false);
				STREAMING::CLEAR_FOCUS();

				ENTITY::FREEZE_ENTITY_POSITION(self::veh, false);
				fcam = false;
			}
		}
		void WInvis(toggleCommand* command)/// broken Only works when enabled when disabled weapon is invis
		{
			if ("Winvis"_TC->get(0).toggle && !winvis)
			{

				ENTITY::SET_ENTITY_VISIBLE(WEAPON::GET_CURRENT_PED_WEAPON_ENTITY_INDEX(PLAYER::PLAYER_PED_ID(), 100), !winvis, false);




				winvis = true;

			}
			if (cPed != nullptr && !"Winvis"_TC->get(0).toggle && winvis)
			{
				ENTITY::SET_ENTITY_VISIBLE(WEAPON::GET_CURRENT_PED_WEAPON_ENTITY_INDEX(PLAYER::PLAYER_PED_ID(), 0), !winvis, true);
				winvis = false;

			}
		}
		void Mradio(toggleCommand* command)/// broken Only works when enabled when disabled weapon is invis
		{
			if ("mradio"_TC->get(0).toggle && !mradio)
			{

				AUDIO::SET_MOBILE_PHONE_RADIO_STATE(true);
				AUDIO::SET_MOBILE_RADIO_ENABLED_DURING_GAMEPLAY(true);




				mradio = true;

			}
			if (cPed != nullptr && !"mradio"_TC->get(0).toggle && mradio)
			{
				AUDIO::SET_MOBILE_PHONE_RADIO_STATE(false);
				AUDIO::SET_MOBILE_RADIO_ENABLED_DURING_GAMEPLAY(false);
				mradio = false;
			}
		}

		void nclub(toggleCommand* command)/// 10 / 10
		{
			if ("nClub"_TC->get(0).toggle && !Nclub)
			{
				static int callCount = 0;
				std::string prefix = "MP" + std::to_string(char_index) + "_";
				STATS::STAT_SET_INT(rage::joaat(prefix + "CLUB_POPULARITY"), 1000, true);
				STATS::STAT_SET_INT(rage::joaat(prefix + "CLUB_PAY_TIME_LEFT"), -1, true);

				if (callCount >= 4) {
					Nclub = true; // Toggle off
					callCount = 0; // Reset call count
				}

			}
			if (cPed != nullptr && !"nClub"_TC->get(0).toggle && mradio)
			{

				Nclub = false;
			}

		}


		void Aimbot(toggleCommand* command)
		{
			if ("aimbot"_TC->get(0).toggle && !aimBot)
			{
				int player = PLAYER::PLAYER_ID();
				int playerPed = PLAYER::PLAYER_PED_ID();

				for (int i = 0; i < 32; i++)
				{
					if (i != player)
					{
						if (GetAsyncKeyState(VK_RBUTTON))
						{
							Ped targetPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
							Vector3 targetPos = ENTITY::GET_ENTITY_COORDS(targetPed, 1);
							BOOL exists = ENTITY::DOES_ENTITY_EXIST(targetPed);
							BOOL dead = PLAYER::IS_PLAYER_DEAD(targetPed);

							if (exists && !dead)
							{
								float screenX, screenY;
								BOOL onScreen = GRAPHICS::GET_SCREEN_COORD_FROM_WORLD_COORD(targetPos.x, targetPos.y, targetPos.z, &screenX, &screenY);
								if (ENTITY::IS_ENTITY_VISIBLE(targetPed) && onScreen)
								{
									if (ENTITY::HAS_ENTITY_CLEAR_LOS_TO_ENTITY(playerPed, targetPed, 17))
									{
										Vector3 targetCoords = PED::GET_PED_BONE_COORDS(targetPed, 31086, 0, 0, 0);
										PED::SET_PED_SHOOTS_AT_COORD(playerPed, targetCoords.x, targetCoords.y, targetCoords.z, 1);

									}
								}
							}
						}
					}
				}

			}
			if (cPed != nullptr && !"aimbot"_TC->get(0).toggle && aimBot)
			{
				ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), true, true);
				aimBot = false;
			}

		}
		void dmoneylocal(toggleCommand* command)/// 10 / 10
		{
			if ("dropm"_TC->get(0).toggle && !DropM)
			{
				static int callCount = 0;
				int moneyValue = 2000;

				// Create money pickup
				Vector3 coords = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true);
				Object pickup = OBJECT::CREATE_AMBIENT_PICKUP(rage::joaat("PICKUP_MONEY_VARIABLE"), coords.x, coords.y, coords.z + 2, moneyValue, 2000, rage::joaat("prop_cash_pile_01"), true, true);

				if (callCount >= 4) {
					DropM = true; // Toggle off
					callCount = 0; // Reset call count
				}

			}
			if (cPed != nullptr && !"dropm"_TC->get(0).toggle && DropM)
			{
				ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), true, true);
				DropM = false;
			}
		}




		enum class PickupModelHash : Hash {
			ACTION_FIGURE = 0x3D1B7A2F, // Example action figure model hash
			// Add more model hashes here if needed
		};

		void Invis(toggleCommand* command)/// 10 / 10
		{
			if ("pinvis"_TC->get(0).toggle && !pInvis)
			{
				static int callCount = 0;
				ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), false, false);

				if (callCount >= 4) {
					pInvis = true; // Toggle off
					callCount = 0; // Reset call count
				}

			}
			if (cPed != nullptr && !"pinvis"_TC->get(0).toggle && pInvis)
			{
				ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), true, true);
				pInvis = false;
			}
		}

		void dPhone(toggleCommand* command)
		{
			if ("DPhone"_TC->get(0).toggle && !dphone)
			{
#define GLOBAL_DISABLE_MOBILE_PHONE		20366
				globalHandle(GLOBAL_DISABLE_MOBILE_PHONE).As<int>() = 1, dphone = true;
				dphone = true;

			}
			if (cPed != nullptr && !"DPhone"_TC->get(0).toggle && dphone)
			{
				globalHandle(GLOBAL_DISABLE_MOBILE_PHONE).As<int>() = 1, dphone = !false;
				dphone = false;
			}



		}
		Vector3 crossProduct(const Vector3& a, const Vector3& b) {
			return Vector3(
				a.y * b.z - a.z * b.y,
				a.z * b.x - a.x * b.z,
				a.x * b.y - a.y * b.x
			);
		}


		void noclip(toggleCommand* command)
		{

			if ("Nclip"_TC->get(0).toggle && !nclip) {
				// Toggle on nclip
				Vector3 position = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true);
				Vector3 speed = ENTITY::GET_ENTITY_SPEED_VECTOR(PLAYER::PLAYER_PED_ID(), true);

				// Handle movement controls
				if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)ControllerInputs::INPUT_SPRINT))
					position += ENTITY::GET_ENTITY_FORWARD_VECTOR(PLAYER::PLAYER_PED_ID()) * 0.1f;
				if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)ControllerInputs::INPUT_DUCK))
					position -= ENTITY::GET_ENTITY_FORWARD_VECTOR(PLAYER::PLAYER_PED_ID()) * 0.1f;
				if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)ControllerInputs::INPUT_MOVE_UP_ONLY))
					position += {0.0f, 0.0f, 0.1f}; // Move up
				if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)ControllerInputs::INPUT_MOVE_DOWN_ONLY))
					position -= {0.0f, 0.0f, 0.1f}; // Move down

				// Compute forward and up vectors
				Vector3 forward = ENTITY::GET_ENTITY_FORWARD_VECTOR(PLAYER::PLAYER_PED_ID());
				Vector3 up = { 0.0f, 0.0f, 1.0f }; // Assuming up vector is along the z-axis

				// Compute right vector using cross product of forward and up vectors
				Vector3 right = {
					forward.y * up.z - forward.z * up.y,
					forward.z * up.x - forward.x * up.z,
					forward.x * up.y - forward.y * up.x
				};

				if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)ControllerInputs::INPUT_MOVE_LEFT_ONLY))
					position -= right * 0.1f;
				if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)ControllerInputs::INPUT_MOVE_RIGHT_ONLY))
					position += right * 0.1f;

				// Set the new position
				ENTITY::SET_ENTITY_COORDS_NO_OFFSET(PLAYER::PLAYER_PED_ID(), position.x, position.y, position.z, false, false, false);
				ENTITY::SET_ENTITY_VELOCITY(PLAYER::PLAYER_PED_ID(), speed.x, speed.y, speed.z);

				ENTITY::SET_ENTITY_COLLISION(PLAYER::PLAYER_PED_ID(), false, false);
			}

			else {
				// Toggle off nclip

				ENTITY::SET_ENTITY_COLLISION(PLAYER::PLAYER_PED_ID(), true, true);

				nclip = false;
			}


		}
		///////////////////////// PROTEX

	}


			


		

		





	

	const std::vector<std::string> g_textures = {
		"commonmenu"
	};

	void request_textures()
	{
		g_pool.add([]
			{
				for (const auto& texture : g_textures)
				{
					while (!GRAPHICS::HAS_STREAMED_TEXTURE_DICT_LOADED(texture.c_str()))
					{
						GRAPHICS::REQUEST_STREAMED_TEXTURE_DICT(texture.c_str(), TRUE);
						LOG(Registers, "Requesting in game textures: {}", texture.c_str())
							fiber::current()->sleep();
					}
				}
			});
	}

	void Not() {
		HUD::BEGIN_TEXT_COMMAND_THEFEED_POST("STRING");
		HUD::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME("~r~Pluto Loaded Successfully");
		HUD::END_TEXT_COMMAND_THEFEED_POST_MESSAGETEXT_WITH_CREW_TAG_AND_ADDITIONAL_ICON("CHAR_LESTER_DEATHWISH", "CHAR_LESTER_DEATHWISH", true, 5, "Welcome To Pluto INS to open menu", "", 1.f, "", 5, 0);
		HUD::END_TEXT_COMMAND_THEFEED_POST_TICKER(false, false);
	}
	void unload_textures()
	{
		g_pool.add([]
			{
				for (const auto& textures : g_textures)
				{
					if (GRAPHICS::HAS_STREAMED_TEXTURE_DICT_LOADED(textures.c_str()))
					{
						GRAPHICS::SET_STREAMED_TEXTURE_DICT_AS_NO_LONGER_NEEDED(textures.c_str());
						LOG(Registers, "Unloading in game textures: {}", textures.c_str())
					}
					else
					{
						LOG(Info, "Unloading without removing any in game textures")
					}
				}
			});
	}


	void init()
	{
		Not();
		
		g_manager.add(toggleCommand("godMode", "GodMode", self::god_mode)); // with super run enabled you die (unless no collision is enabled) 
		g_manager.add(toggleCommand("gracefulLanding", "No Fall Damage", self::graceful_landing)); /// allows you to jump off things and take no damage 
		g_manager.add(toggleCommand("beastJump", "Beast Jump", self::beast_jump)); // descibes what it dose already
		g_manager.add(toggleCommand("uammo", "Unlimited Ammo", self::Uammo));// no issues unlim ammo
		g_manager.add(toggleCommand("nWanted", "never Wanted", self::neverWanted)); // descibes what it dose already
		g_manager.add(toggleCommand("slowmode", "Slow Motion", self::SlowMode));

		g_manager.add(toggleCommand("srun", "Super Run", self::SuperRun)); // descibes what it dose already 
		g_manager.add(toggleCommand("sswim", "Super Run", self::Superswim)); // descibes what it dose already
		g_manager.add(toggleCommand("uox", "Unlimited Oxegen", self::UnlimitedOxegen));
		g_manager.add(toggleCommand("ndoll", "No Ragdoll", self::Noragdoll)); /// issue while Whater phiscis enabled you die
		g_manager.add(toggleCommand("nwcollision", "No Water collision", self::Nowcollision)); /// 10/10 issue with No Ragdoll
		g_manager.add(toggleCommand("aWanted", "Alwayys Wanted", self::alwaysWanted)); // always wanted dose what it says
		g_manager.add(toggleCommand("vGmode", "Vehicle Godmode", self::VehGodmode)); // works fine no issues 
		g_manager.add(toggleCommand("ghmode", "Invisable", self::Ghost)); // i like this 
		g_manager.add(toggleCommand("dMulti", "Damage Multiplier", self::dMg)); // 1 hit 
		g_manager.add(toggleCommand("eammo", "Explosive Ammo", self::ExploAmmo)); // not working causes crashing // no longer crashes only works in story mode tho!
		g_manager.add(toggleCommand("tinyp", "Tiny Player", self::Tplayer)); /// ONLY LOCAL


		g_manager.add(toggleCommand("pedfly", "Vehicle Gravity", self::pEdfly)); // sometimes dosnt work
		g_manager.add(toggleCommand("Winvis", "Weapon Invisability", self::WInvis)); // broken atm needs fixed (toggles only when off) 

		g_manager.add(toggleCommand("mradio", "Mobile Radio", self::Mradio)); /// needs looped mabey? (NVM IT WORKS1)

		g_manager.add(toggleCommand("fCam", "Free Cam", self::Fcam)); /// chat GPT FTW
		g_manager.add(toggleCommand("DPhone", "Dsiable Phone", self::dPhone)); /// causes freezing dosnt work outdated globals???
		g_manager.add(toggleCommand("Nclip", "Noclip", self::noclip)); /// works controlls are wonky
		g_manager.add(toggleCommand("nClub", "Nightclub safe", self::nclub)); /// Probably DTC  not sure may cause crashing / works fine testing if dtc
		g_manager.add(toggleCommand("dropm", "Money drop? ", self::dmoneylocal)); /// CREDITS https://www.unknowncheats.me/forum/grand-theft-auto-v/621670-money-drops-yourself-yimmenu.html // possibly use the function for rp?
		g_manager.add(toggleCommand("pinvis", "Player invis?", self::Invis)); /// FIXED
		g_manager.add(toggleCommand("rman", "Rocket man", self::Rocketman)); /// holy shit this is fun
		g_manager.add(toggleCommand("esp", "esp", self::esp));
		
		
		g_manager.add(toggleCommand("aimbot", "aimbot", self::Aimbot)); /// bit buggy causes issues when injecting into sp NO LONGER
		

		
	

		
		


	






	}

	void uninit()
	{
		const auto ped{ PLAYER::PLAYER_PED_ID() };
		ENTITY::SET_ENTITY_COLLISION(ped, TRUE, TRUE);
		ENTITY::RESET_ENTITY_ALPHA(ped);
		ENTITY::SET_ENTITY_VISIBLE(ped, TRUE, FALSE);
		ENTITY::SET_ENTITY_HAS_GRAVITY(ped, TRUE);
		PLAYER::SET_MAX_WANTED_LEVEL(5);
		PLAYER::SET_POLICE_RADAR_BLIPS(TRUE);
		const auto player{ PLAYER::PLAYER_ID() };
		PLAYER::RESET_WANTED_LEVEL_HIDDEN_ESCAPE_TIME(player);
		unload_textures();
	}

	void on_init()
	{	
		
		request_textures();
		
		//These need to be after init because the values aren't created yet
	}

	void on_tick()
	{
		cPed = util::classes::getPed();
		cPedWeaponManager = cPed->m_weapon_manager;
		cWeaponInfo = cPedWeaponManager->m_weapon_info;
		cAmmoInfo = cWeaponInfo->m_ammo_info;
		cVehicle = cPed->m_vehicle;
		cPlayerInfo = util::classes::getPlayerInfo();
	}
}
