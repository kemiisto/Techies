#ifndef  TECHIES_APP_DELEGATE_H
#define  TECHIES_APP_DELEGATE_H

#include "CCApplication.h"

class AppDelegate final : cocos2d::Application {
public:
    AppDelegate() = default;
    ~AppDelegate() override = default;
	
    AppDelegate(const AppDelegate&) = delete;
    AppDelegate& operator=(const AppDelegate&) = delete;

    AppDelegate(AppDelegate&&) = delete;
    AppDelegate& operator=(AppDelegate&&) = delete;

    void initGLContextAttrs() override;
    bool applicationDidFinishLaunching() override;
    void applicationDidEnterBackground() override;
    void applicationWillEnterForeground() override;
};

#endif // TECHIES_APP_DELEGATE_H

