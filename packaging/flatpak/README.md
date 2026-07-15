# BLOODRUSH — Flatpak

## Локальная сборка и установка

```sh
# один раз: инструменты и рантайм
flatpak remote-add --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo
flatpak install -y flathub org.freedesktop.Platform//24.08 org.freedesktop.Sdk//24.08
# flatpak-builder: из пакетного менеджера или org.flatpak.Builder

# сборка + установка для текущего пользователя
flatpak-builder --user --install --force-clean build-dir \
    packaging/flatpak/io.github.shidikusik.Bloodrush.yml

flatpak run io.github.shidikusik.Bloodrush
```

## Готовый .flatpak из CI

GitHub Actions собирает бандл на каждый пуш: вкладка **Actions** →
последний запуск **Flatpak** → артефакт `bloodrush.flatpak`.

```sh
flatpak install --user bloodrush.flatpak
```

## Публикация на Flathub (делает владелец репозитория)

1. Создать релизный тег (манифест ссылается на `v0.1.0`).
2. Форкнуть https://github.com/flathub/flathub, создать ветку
   `new-app-io.github.shidikusik.Bloodrush` от ветки `new-pr`.
3. Положить в корень форка `io.github.shidikusik.Bloodrush.yml`
   (этот манифест), добавив к git-источнику строку `commit: <sha тега>`.
4. Открыть PR в flathub/flathub против ветки `new-pr` и пройти ревью.
   Подробности: https://docs.flathub.org/docs/for-app-authors/submission

После принятия приложение появится в магазине, а обновления делаются
пушем в созданный Flathub'ом репозиторий flathub/io.github.shidikusik.Bloodrush.
