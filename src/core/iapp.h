#ifndef __IAPP__H__
#define __IAPP__H__

#include <chrono>

namespace gefx
{
	class IApp
	{
		using highResClock = std::chrono::high_resolution_clock;

	  public:
		constexpr explicit IApp(const char* name)
			: _shouldSleep(false), _shouldWakeUp(true), _shouldQuit(false), name(name), _sleeping(false),
			  _deltaTime(0.016){};
		virtual ~IApp() = default;

		void Run();
		const char* GetName() { return name; };

		IApp() = delete;
		IApp(IApp&&) = delete;
		IApp(const IApp&) = delete;
		IApp& operator=(IApp&&) = delete;
		IApp& operator=(const IApp&) = delete;

	  protected:
		virtual void Setup() = 0;
		virtual void Awake() = 0;
		virtual void Update(double deltaTime) = 0;
		virtual void Sleep() = 0;
		virtual void Shutdown() = 0;

		bool _shouldSleep;
		bool _shouldWakeUp;
		bool _shouldQuit;

	  private:
		const char* name;
		bool _sleeping;
		double _deltaTime;
	};

	inline void IApp::Run()
	{
		Setup();
		while (!_shouldQuit)
		{
			if (_sleeping)
			{
				if (_shouldWakeUp)
				{
					_sleeping = false;
					_shouldWakeUp = false;
					Awake();
					continue;
				}
			}
			else
			{
				if (_shouldSleep)
				{
					Sleep();
					_sleeping = true;
					_shouldSleep = false;
					continue;
				}
			}

			highResClock::time_point start = highResClock::now();
			Update(_deltaTime);
			highResClock::time_point stop = highResClock::now();

			using ms = std::chrono::duration<double>;
			_deltaTime = std::chrono::duration_cast<ms>(stop - start).count();
		}
		Shutdown();
	}
} // namespace gefx

#endif //!__IAPP__H__