# BLOODRUSH → Steam: пошаговый гайд

Всё техническое уже готово в репозитории:

| Что | Где |
|---|---|
| Windows-сборка для депота | CI-артефакт `bloodrush-windows-x86_64` (workflow **Builds**) |
| Linux-сборка под Steam Runtime (sniper) | CI-артефакт `bloodrush-steam-linux` (workflow **Builds**) |
| Скрипт загрузки депотов | `steam/app_build.vdf` |
| Заготовки графики магазина | `steam/store_assets/` |
| Скриншоты (1280×720, подходят) | `docs/screenshots/` |

Дальше — шаги, которые может сделать **только владелец аккаунта** (вы).

---

## Шаг 1. Аккаунт Steamworks (~1–3 дня)

1. Зайдите на https://partner.steamgames.com → **Join Steamworks**.
2. Заполните данные компании/физлица, налоговую информацию
   (интервью W-8BEN для не-США) и банковские реквизиты для выплат.
   ⚠️ Выплаты Valve идёт банковским переводом — проверьте, что ваш
   банк/страна принимает переводы от Valve (для РФ сейчас это проблема;
   рабочие обходные пути — счёт в другой стране или партнёр-издатель).
3. Оплатите **Steam Direct fee — $100** за слот приложения
   (возвращается после $1000 выручки).

## Шаг 2. Создание приложения

1. В Steamworks: **Create New App** → получите **AppID** (например 3457890).
2. Steam автоматически создаст два депота: `AppID+1` и `AppID+2`.
3. В `steam/app_build.vdf` замените `YOUR_APP_ID`, `YOUR_APP_ID_PLUS_1`,
   `YOUR_APP_ID_PLUS_2` на реальные числа.

## Шаг 3. Страница магазина

Steamworks → App → **Store Presence**:

- **Тексты**: короткое описание (~300 знаков) и полное — можно взять из
  README; поддерживаются оба языка (Russian + English у нас уже есть).
- **Графика**: загрузите файлы из `steam/store_assets/`
  (это заготовки в правильных размерах — для релиза стоит заказать/сделать
  арт покрасивее, но для Coming Soon страницы они валидны):
  - header_capsule 460×215, small 231×87, main 616×353 (+2x 1232×706),
    vertical 374×448, library 600×900, hero 3840×1240, logo (прозрачный).
- **Скриншоты**: минимум 5 штук. Наши в `docs/screenshots/` подходят
  (1280×720); лучше наснимать свежих из финальной версии (F11 + PrtScr).
- **Трейлер**: обязателен для нормальной видимости. 30–60 секунд геймплея,
  захваченного OBS, хватит.
- **Теги/жанры**: Action, FPS, Arena Shooter, Fast-Paced, Retro, Indie.
- **Возрастной опрос**: у нас пиксельная кровь → отметьте
  "Frequent Violence or Gore" честно, ничего страшного.
- **Системные требования**: минимум — любой GPU с OpenGL 3.3,
  2 ГБ RAM, 100 МБ на диске. Windows 10+, Linux (Steam Runtime).

## Шаг 4. Настройка сборки (Steamworks → Installation)

**General Installation → Launch Options** — создайте две записи:

| # | Executable | Operating System |
|---|---|---|
| 1 | `bloodrush.exe` | Windows |
| 2 | `bloodrush` | Linux + SteamOS |

**SteamPipe → Builds**: после первой загрузки билда выберите его
и нажмите **Set build live on default branch**.

**Linux Runtime**: в разделе Installation → Linux Runtime выберите
**Steam Linux Runtime 3.0 (sniper)** — наш бинарь собран именно под него.

## Шаг 5. Загрузка билдов

На своей машине (Linux):

```sh
# 1) Скачайте steamcmd
mkdir -p ~/steamcmd && cd ~/steamcmd
curl -sqL https://steamcdn-a.akamaihd.net/client/installer/steamcmd_linux.tar.gz | tar xz

# 2) Скачайте артефакты из GitHub Actions (последний зелёный прогон Builds):
#    bloodrush-windows-x86_64 -> распакуйте bloodrush.exe
#    bloodrush-steam-linux    -> распакуйте bloodrush
#    и разложите в репозитории:
#    steam/content/windows/bloodrush.exe
#    steam/content/linux/bloodrush   (chmod +x bloodrush)

# 3) Загрузка (из каталога steam/ репозитория)
~/steamcmd/steamcmd.sh +login ВАШ_ЛОГИН +run_app_build $(pwd)/app_build.vdf +quit
```

steamcmd спросит Steam Guard код при первом входе. После загрузки билд
появится в Steamworks → SteamPipe → Builds — назначьте его на default.

## Шаг 6. Проверка

1. В клиенте Steam: Библиотека → игра появится у владельца приложения.
2. Проверьте установку/запуск на Windows и Linux (или Steam Deck).
3. Полезно: включите себе бета-ветку и позовите пару друзей ключами
   (Steamworks → Request Steam Keys).

## Шаг 7. Релиз

1. **Coming Soon страница должна провисеть минимум 2 недели** до релиза —
   опубликуйте её как можно раньше (проверка Valve: 2–5 рабочих дней).
2. Отдельно на проверку отправляется **билд** (тоже 1–5 дней).
3. Назначьте цену (или Free to Play) в разделе Pricing.
4. В день релиза — кнопка **Release App**. Всё.

## Что можно добавить потом (не блокирует релиз)

- **Достижения / облачные сейвы / оверлей-статистика** — требуют
  подключения Steamworks SDK (закрытая лицензия, в публичный репозиторий
  его класть нельзя — подключается локально). Игра прекрасно живёт и без.
- **Steam Deck Verified** — наш Linux-билд под sniper уже правильный
  кандидат; после релиза Valve сама проверит, можно ускорить запросом.
- **Trading Cards** — доступны после $1000 выручки.

## Чеклист одной строкой

☐ Steamworks аккаунт + $100 → ☐ AppID в `app_build.vdf` →
☐ Store page (тексты/арт/скриншоты/трейлер) → ☐ Launch options + sniper →
☐ steamcmd upload → ☐ Set live → ☐ тест на Win/Linux →
☐ Coming Soon ≥2 недели → ☐ ревью Valve → ☐ Release.
