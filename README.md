# Techies
Простенькая игра на Cocos2d-x по мотивам Dota 2. :D
- Вы играете за Techies, способного устанавливать на игровое поле две различные мины: красную, которая автоматически взорвется при приближении врага и зелёную, которая взрывается только при активации тапом по ней вручную. 
- Ваша задача - пропустить как можно меньше наступающих вражеских крипов, используя мины. Для установки мины необходимо просто перетащить её на игровое поле.
- Пропущенные крипы наносят урон (условно, строениям, которые защищает Techies, находящимся за пределами экрана), взорванные крипы приносят очки.
- Крипы бывают 3 типов:
  - Крип ближнего боя (крип-воин с мечом в руках). Наносит 10% урона, за подрыв игрок получает 1 очко.
  - Крип дальнего боя (крип-маг с посохом в руках). Наносит 20% урона, за подрыв игрок получает 2 очка.
  - Катапульта (осадная тележка). Наносит 30% урона, за подрыв игрок получает 3 очка.
- Игра прекращается, как только крипы нанесут суммарно 100% урона.

[![](https://j.gifs.com/NOxDjm.gif)](https://youtu.be/m7RbxXWPCR0)

Скачать v0.3.1 Beta:
- [macOS](https://github.com/kemiisto/Techies/releases/download/v0.3.1/Techies.macOS.zip)
- [Windows](https://github.com/kemiisto/Techies/releases/download/v0.3.1/Techies.Windows.zip)

Build instructions:
- Windows
  - `cd proj.win32`
  - `cmake .. -G "Visual Studio 16 2019" -AWin32`
  - Open Techies.sln, set Techies as startup project.
  - Open Techies project properties, go to C/C++ -> Language and change "C++ Language Standard" value to "ISO C++17 Standard (/std:c++17)".
- macOS
  - `cd proj.ios_mac/mac`
  - `cmake ../.. -GXcode`
  - Open Techies.xcodeproj, set Techies as active scheme.
  - Open Techies project properties and change the following Build settings for Techies target: in Apple Clang - Custom Compiler Flags -> Other C++ Flags change all `-std=c++11` entries to `-std=c++17`.
