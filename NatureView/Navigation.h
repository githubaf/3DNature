
#ifndef NVW_NAVIGATION_H
#define NVW_NAVIGATION_H


class Navigation
	{
	public:
		enum NavigationMode
			{
			NAV_NM_DRIVE = 0,
			NAV_NM_SLIDE,
			NAV_NM_ROTATE,
			NAV_NM_CLIMB,
			NAV_NM_QUERY,
			NAV_NM_MAX // no comma
			}; // NavigationMode
	private:
		static float CurrentSpeed;
		static NavigationMode CurrentNavMode;
	public:

		Navigation() {/* CurrentNavMode = NAV_NM_DRIVE; CurrentSpeed = 1.0f; */}; // this probably won't really get used due to the static nature of the pseudo-singleton
		~Navigation() {};

		static float GetCurrentSpeed(void);
		static void SetCurrentSpeed(float NewValue) {CurrentSpeed = NewValue;};
		static NavigationMode GetCurrentNavMode(void) {return(CurrentNavMode);};
		static void SetCurrentNavMode(NavigationMode NewValue) {CurrentNavMode = NewValue;};

	}; // Navigation

#endif // NVW_NAVIGATION_H
