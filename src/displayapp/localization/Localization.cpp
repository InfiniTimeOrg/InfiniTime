#include "displayapp/localization/Localization.h"

using namespace Pinetime::Applications::Localization;

namespace {
  using TranslationTable = std::array<const char*, StringCount>;

  constexpr TranslationTable english {{
    "",
    "Settings",
    "Display",
    "Wake Up",
    "Time format",
    "Watch face",
    "Steps",
    "Heartrate",
    "Date & Time",
    "Weather",
    "Battery",
    "Chimes",
    "Shake Calib.",
    "Firmware",
    "Over-the-air",
    "Bluetooth",
    "About",
    "Language",
    "English",
    "Spanish",
    "Portuguese",
    "Russian",
    "French",
    "German",
    "Italian",
    "Turkish",
    "Polish",
    "Catalan",
    "Basque",
    "Back",
    "On",
    "Off",
    "Enabled",
    "Disabled",
    "Start",
    "Stop",
    "Pause",
    "Reset",
    "Set",
    "Charging",
    "Fully charged",
    "Discharging",
    "Battery low",
    "Battery critical",
    "Reading Battery",
    "volts",
    "Daily steps goal",
    "Goal",
    "Trip",
    "Yest",
    "Heart rate BPM",
    "Not enough data,\nplease wait...",
    "No touch detected",
    "Measuring...",
    "Stopped",
    "Backg. Interval",
    "Cont",
    "Display timeout",
    "Always On",
    "Single Tap",
    "Double Tap",
    "Raise Wrist",
    "Shake Wake",
    "Lower Wrist",
    "Set current time",
    "Set current date",
    "Metric",
    "Imperial",
    "Every hour",
    "Every 30 mins",
    "Till reboot",
    "Firmware & files",
    "Clear sky",
    "Few clouds",
    "Scattered clouds",
    "Broken clouds",
    "Shower rain",
    "Rain",
    "Thunderstorm",
    "Snow",
    "Mist",
    "Clear",
    "Cloudy",
    "Showers",
    "Thunder",
    "Notification",
    "Incoming call from",
    "Notifications",
    "No notifications",
    "too large",
    "zero division",
    "Warning",
    "Touch controller error detected.",
    "If you encounter problems and your device is under warranty, contact the devices seller.",
    "Proceed",
    "Version",
    "Short Ref",
    "This firmware has\nbeen #00ff00 validated#",
    "Any reboot will\nrollback to last\nvalidated firmware",
    "Validate",
    "Rollback",
    "Firmware update",
    "Waiting...",
    "Wake Sensitivity",
    "Calibrate",
    "Shake!",
    "Ready!",
    "Connected",
    "Disconnected",
    "Image OK!",
    "Error!",
    "You have\nmail!",
    "Navigation",
    "Score",
    "Alarm\nis not\nset.",
    "Time to\nalarm:",
    "Days",
    "Hours",
    "Minutes",
    "Seconds",
    "Once",
    "Daily",
    "Mon-Fri",
    "Heads",
    "Tails",
  }};

  constexpr TranslationTable spanish {{
    "",
    "Ajustes",
    "Pantalla",
    "Activar",
    "Formato hora",
    "Esfera",
    "Pasos",
    "Pulso",
    "Fecha y hora",
    "Clima",
    "Batería",
    "Avisos",
    "Calibrar mov.",
    "Firmware",
    "OTA",
    "Bluetooth",
    "Acerca de",
    "Idioma",
    "Inglés",
    "Español",
    "Portugués",
    "Ruso",
    "Francés",
    "Alemán",
    "Italiano",
    "Turco",
    "Polaco",
    "Catalán",
    "Euskera",
    "Atrás",
    "Act.",
    "Des.",
    "Activo",
    "Inactivo",
    "Iniciar",
    "Parar",
    "Pausa",
    "Reiniciar",
    "Fijar",
    "Cargando",
    "Carga compl.",
    "Descargando",
    "Batería baja",
    "Bat. crítica",
    "Leyendo batería",
    "voltios",
    "Meta diaria",
    "Meta",
    "Viaje",
    "Ayer",
    "Pulso LPM",
    "Faltan datos,\nespera...",
    "Sin contacto",
    "Midiendo...",
    "Parado",
    "Interv. fondo",
    "Cont",
    "Espera pantalla",
    "Siempre act.",
    "Toque simple",
    "Doble toque",
    "Alzar muñeca",
    "Agitar",
    "Bajar muñeca",
    "Ajustar hora",
    "Ajustar fecha",
    "Métrico",
    "Imperial",
    "Cada hora",
    "Cada 30 min",
    "Hasta reinicio",
    "Firmware y arch.",
    "Despejado",
    "Pocas nubes",
    "Nubes dispersas",
    "Muy nuboso",
    "Chubascos",
    "Lluvia",
    "Tormenta",
    "Nieve",
    "Niebla",
    "Despej.",
    "Nublado",
    "Chubasc.",
    "Trueno",
    "Notificación",
    "Llamada de",
    "Notificaciones",
    "Sin notificaciones",
    "muy grande",
    "división cero",
    "Aviso",
    "Error táctil detectado.",
    "Si hay problemas y está en garantía, contacta con el vendedor.",
    "Continuar",
    "Versión",
    "Ref corta",
    "Firmware\n#00ff00 validado#",
    "Al reiniciar\nvolverá al firmware\nvalidado anterior",
    "Validar",
    "Revertir",
    "Act. firmware",
    "Esperando...",
    "Sensibilidad",
    "Calibrar",
    "¡Agita!",
    "¡Listo!",
    "Conectado",
    "Desconectado",
    "¡Imagen OK!",
    "¡Error!",
    "¡Tienes\ncorreo!",
    "Navegación",
    "Puntuación",
    "Alarma\nno\nfijada.",
    "Tiempo hasta\nalarma:",
    "Días",
    "Horas",
    "Minutos",
    "Segundos",
    "Una vez",
    "Diario",
    "Lun-Vie",
    "Cara",
    "Cruz",
  }};

  constexpr std::array<TranslationTable, LanguageCount> translations {{
    english,
    spanish,
    english,
    english,
    english,
    english,
    english,
    english,
    english,
    english,
    english,
  }};

  static_assert(english.size() == StringCount);
  static_assert(spanish.size() == StringCount);
  static_assert(translations.size() == LanguageCount);

  const char* TranslatePortuguese(StringId id) {
    switch (id) {
      case StringId::Settings:
        return "Configuracoes";
      case StringId::Display:
        return "Tela";
      case StringId::WakeUp:
        return "Despertar";
      case StringId::TimeFormat:
        return "Formato hora";
      case StringId::WatchFace:
        return "Mostrador";
      case StringId::Steps:
        return "Passos";
      case StringId::HeartRate:
        return "Pulso";
      case StringId::DateTime:
        return "Data e hora";
      case StringId::Weather:
        return "Tempo";
      case StringId::Battery:
        return "Bateria";
      case StringId::Chimes:
        return "Sinos";
      case StringId::About:
        return "Sobre";
      case StringId::Language:
        return "Idioma";
      case StringId::English:
        return "Ingles";
      case StringId::Spanish:
        return "Espanhol";
      case StringId::Portuguese:
        return "Portugues";
      case StringId::Russian:
        return "Russo";
      case StringId::French:
        return "Frances";
      case StringId::German:
        return "Alemao";
      case StringId::Italian:
        return "Italiano";
      case StringId::Turkish:
        return "Turco";
      case StringId::Polish:
        return "Polones";
      case StringId::Catalan:
        return "Catalao";
      case StringId::Basque:
        return "Euskera";
      case StringId::Back:
        return "Voltar";
      case StringId::On:
        return "Lig";
      case StringId::Off:
        return "Desl";
      case StringId::Enabled:
        return "Ativado";
      case StringId::Disabled:
        return "Desativado";
      case StringId::Start:
        return "Iniciar";
      case StringId::Stop:
        return "Parar";
      case StringId::Pause:
        return "Pausa";
      case StringId::Reset:
        return "Redefinir";
      case StringId::Set:
        return "Definir";
      case StringId::Charging:
        return "Carregando";
      case StringId::FullyCharged:
        return "Carga total";
      case StringId::Discharging:
        return "Descarregando";
      case StringId::BatteryLow:
        return "Bateria fraca";
      case StringId::BatteryCritical:
        return "Bateria critica";
      case StringId::ReadingBattery:
        return "Lendo bateria";
      case StringId::Volts:
        return "volts";
      case StringId::DailyStepsGoal:
        return "Meta diaria";
      case StringId::Goal:
        return "Meta";
      case StringId::Trip:
        return "Percurso";
      case StringId::Yesterday:
        return "Ontem";
      case StringId::HeartRateBpm:
        return "Pulso BPM";
      case StringId::NotEnoughData:
        return "Dados insuficientes,\naguarde...";
      case StringId::NoTouchDetected:
        return "Sem contato";
      case StringId::Measuring:
        return "Medindo...";
      case StringId::Stopped:
        return "Parado";
      case StringId::BackgroundInterval:
        return "Interv. fundo";
      case StringId::Continuous:
        return "Continuo";
      case StringId::DisplayTimeout:
        return "Tempo da tela";
      case StringId::AlwaysOn:
        return "Sempre ativa";
      case StringId::SetCurrentTime:
        return "Ajustar hora";
      case StringId::SetCurrentDate:
        return "Ajustar data";
      case StringId::Metric:
        return "Metrico";
      case StringId::Imperial:
        return "Imperial";
      case StringId::EveryHour:
        return "Cada hora";
      case StringId::Every30Minutes:
        return "Cada 30 min";
      case StringId::TillReboot:
        return "Ate reiniciar";
      case StringId::FirmwareAndFiles:
        return "Firmware e arquivos";
      case StringId::ClearSky:
        return "Ceu limpo";
      case StringId::FewClouds:
        return "Poucas nuvens";
      case StringId::ScatteredClouds:
        return "Nuvens dispersas";
      case StringId::BrokenClouds:
        return "Muito nublado";
      case StringId::ShowerRain:
        return "Pancadas";
      case StringId::Rain:
        return "Chuva";
      case StringId::Thunderstorm:
        return "Tempestade";
      case StringId::Snow:
        return "Neve";
      case StringId::Mist:
        return "Neblina";
      case StringId::Clear:
        return "Limpo";
      case StringId::Cloudy:
        return "Nublado";
      case StringId::Showers:
        return "Pancadas";
      case StringId::Thunder:
        return "Trovao";
      case StringId::Notification:
        return "Notificacao";
      case StringId::IncomingCallFrom:
        return "Chamada de";
      case StringId::Notifications:
        return "Notificacoes";
      case StringId::NoNotifications:
        return "Sem notificacoes";
      case StringId::TooLarge:
        return "Grande demais";
      case StringId::ZeroDivision:
        return "Divisao por zero";
      case StringId::Warning:
        return "Aviso";
      case StringId::TouchControllerError:
        return "Erro no toque detectado.";
      case StringId::WarrantyTip:
        return "Se houver problemas e estiver na garantia, contacte o vendedor.";
      case StringId::Proceed:
        return "Continuar";
      case StringId::Version:
        return "Versao";
      case StringId::FirmwareValidated:
        return "Este firmware foi\n#00ff00 validado#";
      case StringId::FirmwareRollbackWarning:
        return "Reiniciar vai\nvoltar ao ultimo\nfirmware validado";
      case StringId::Validate:
        return "Validar";
      case StringId::Rollback:
        return "Reverter";
      case StringId::FirmwareUpdate:
        return "Atualizacao firmware";
      case StringId::Waiting:
        return "Aguardando...";
      case StringId::WakeSensitivity:
        return "Sensib. despertar";
      case StringId::Calibrate:
        return "Calibrar";
      case StringId::Shake:
        return "Agite!";
      case StringId::Ready:
        return "Pronto!";
      case StringId::Connected:
        return "Conectado";
      case StringId::Disconnected:
        return "Desconectado";
      case StringId::ImageOk:
        return "Imagem OK!";
      case StringId::Error:
        return "Erro!";
      case StringId::YouHaveMail:
        return "Voce tem\ncorreio!";
      case StringId::Navigation:
        return "Navegacao";
      case StringId::Score:
        return "Pontos";
      case StringId::AlarmNotSet:
        return "Alarme\nnao esta\nativo.";
      case StringId::TimeToAlarm:
        return "Tempo para\no alarme:";
      case StringId::Days:
        return "Dias";
      case StringId::Hours:
        return "Horas";
      case StringId::Minutes:
        return "Minutos";
      case StringId::Seconds:
        return "Segundos";
      case StringId::Once:
        return "Uma vez";
      case StringId::Daily:
        return "Diario";
      case StringId::Weekdays:
        return "Seg-Sex";
      case StringId::Heads:
        return "Cara";
      case StringId::Tails:
        return "Coroa";
      default:
        return nullptr;
    }
  }

  const char* TranslateRussian(StringId id) {
    switch (id) {
      case StringId::Settings:
        return "Настройки";
      case StringId::Display:
        return "Экран";
      case StringId::WakeUp:
        return "Пробуждение";
      case StringId::TimeFormat:
        return "Формат времени";
      case StringId::WatchFace:
        return "Циферблат";
      case StringId::Steps:
        return "Шаги";
      case StringId::HeartRate:
        return "Пульс";
      case StringId::DateTime:
        return "Дата и время";
      case StringId::Weather:
        return "Погода";
      case StringId::Battery:
        return "Батарея";
      case StringId::Chimes:
        return "Сигналы";
      case StringId::About:
        return "О системе";
      case StringId::Language:
        return "Язык";
      case StringId::English:
        return "Английский";
      case StringId::Spanish:
        return "Испанский";
      case StringId::Portuguese:
        return "Португальский";
      case StringId::Russian:
        return "Русский";
      case StringId::French:
        return "Французский";
      case StringId::German:
        return "Немецкий";
      case StringId::Italian:
        return "Итальянский";
      case StringId::Turkish:
        return "Турецкий";
      case StringId::Polish:
        return "Польский";
      case StringId::Catalan:
        return "Каталанский";
      case StringId::Basque:
        return "Баскский";
      case StringId::Back:
        return "Назад";
      case StringId::On:
        return "Вкл";
      case StringId::Off:
        return "Выкл";
      case StringId::Enabled:
        return "Включено";
      case StringId::Disabled:
        return "Выключено";
      case StringId::Start:
        return "Пуск";
      case StringId::Stop:
        return "Стоп";
      case StringId::Pause:
        return "Пауза";
      case StringId::Reset:
        return "Сброс";
      case StringId::Set:
        return "Уст.";
      case StringId::Charging:
        return "Зарядка";
      case StringId::FullyCharged:
        return "Заряжено";
      case StringId::Discharging:
        return "Разрядка";
      case StringId::BatteryLow:
        return "Низкий заряд";
      case StringId::BatteryCritical:
        return "Критичный заряд";
      case StringId::ReadingBattery:
        return "Чтение батареи";
      case StringId::Volts:
        return "вольт";
      case StringId::DailyStepsGoal:
        return "Цель шагов";
      case StringId::Goal:
        return "Цель";
      case StringId::Trip:
        return "Сессия";
      case StringId::Yesterday:
        return "Вчера";
      case StringId::HeartRateBpm:
        return "Пульс уд/м";
      case StringId::NotEnoughData:
        return "Недостаточно данных,\nподождите...";
      case StringId::NoTouchDetected:
        return "Нет касания";
      case StringId::Measuring:
        return "Измерение...";
      case StringId::Stopped:
        return "Остановлено";
      case StringId::BackgroundInterval:
        return "Интервал фона";
      case StringId::Continuous:
        return "Непрерывно";
      case StringId::DisplayTimeout:
        return "Таймаут экрана";
      case StringId::AlwaysOn:
        return "Всегда вкл.";
      case StringId::SetCurrentTime:
        return "Установить время";
      case StringId::SetCurrentDate:
        return "Установить дату";
      case StringId::Metric:
        return "Метрич.";
      case StringId::Imperial:
        return "Имперск.";
      case StringId::EveryHour:
        return "Каждый час";
      case StringId::Every30Minutes:
        return "Каждые 30 мин";
      case StringId::TillReboot:
        return "До перезагр.";
      case StringId::FirmwareAndFiles:
        return "Firmware и файлы";
      case StringId::ClearSky:
        return "Ясно";
      case StringId::FewClouds:
        return "Мало облаков";
      case StringId::ScatteredClouds:
        return "Облака";
      case StringId::BrokenClouds:
        return "Пасмурно";
      case StringId::ShowerRain:
        return "Ливень";
      case StringId::Rain:
        return "Дождь";
      case StringId::Thunderstorm:
        return "Гроза";
      case StringId::Snow:
        return "Снег";
      case StringId::Mist:
        return "Туман";
      case StringId::Clear:
        return "Ясно";
      case StringId::Cloudy:
        return "Облачно";
      case StringId::Showers:
        return "Ливни";
      case StringId::Thunder:
        return "Гром";
      case StringId::Notification:
        return "Уведомление";
      case StringId::IncomingCallFrom:
        return "Входящий вызов от";
      case StringId::Notifications:
        return "Уведомления";
      case StringId::NoNotifications:
        return "Нет уведомлений";
      case StringId::TooLarge:
        return "Слишком большое";
      case StringId::ZeroDivision:
        return "Деление на ноль";
      case StringId::Warning:
        return "Внимание";
      case StringId::TouchControllerError:
        return "Ошибка сенсора.";
      case StringId::WarrantyTip:
        return "Если часы на гарантии и есть проблемы, обратитесь к продавцу.";
      case StringId::Proceed:
        return "Продолжить";
      case StringId::Version:
        return "Версия";
      case StringId::FirmwareValidated:
        return "Эта версия\n#00ff00 подтверждена#";
      case StringId::FirmwareRollbackWarning:
        return "Любая перезагрузка\nвернет последнюю\nподтвержденную версию";
      case StringId::Validate:
        return "Подтв.";
      case StringId::Rollback:
        return "Откат";
      case StringId::FirmwareUpdate:
        return "Обновление ПО";
      case StringId::Waiting:
        return "Ожидание...";
      case StringId::WakeSensitivity:
        return "Чувств. жеста";
      case StringId::Calibrate:
        return "Калибр.";
      case StringId::Shake:
        return "Встряхните!";
      case StringId::Ready:
        return "Готово!";
      case StringId::Connected:
        return "Подключено";
      case StringId::Disconnected:
        return "Отключено";
      case StringId::ImageOk:
        return "Образ OK!";
      case StringId::Error:
        return "Ошибка!";
      case StringId::YouHaveMail:
        return "У вас\nпочта!";
      case StringId::Navigation:
        return "Навигация";
      case StringId::Score:
        return "Счет";
      case StringId::AlarmNotSet:
        return "Будильник\nне\nустановлен.";
      case StringId::TimeToAlarm:
        return "До сигнала:";
      case StringId::Days:
        return "Дней";
      case StringId::Hours:
        return "Часов";
      case StringId::Minutes:
        return "Минут";
      case StringId::Seconds:
        return "Секунд";
      case StringId::Once:
        return "Один раз";
      case StringId::Daily:
        return "Ежедневно";
      case StringId::Weekdays:
        return "Пн-Пт";
      case StringId::Heads:
        return "Орел";
      case StringId::Tails:
        return "Решка";
      default:
        return nullptr;
    }
  }

  const char* TranslateFrench(StringId id) {
    switch (id) {
      case StringId::Settings:
        return "Parametres";
      case StringId::Display:
        return "Affichage";
      case StringId::WakeUp:
        return "Reveil";
      case StringId::TimeFormat:
        return "Format heure";
      case StringId::WatchFace:
        return "Cadran";
      case StringId::Steps:
        return "Pas";
      case StringId::HeartRate:
        return "Frequence";
      case StringId::DateTime:
        return "Date et heure";
      case StringId::Weather:
        return "Meteo";
      case StringId::Battery:
        return "Batterie";
      case StringId::Chimes:
        return "Carillon";
      case StringId::About:
        return "A propos";
      case StringId::Language:
        return "Langue";
      case StringId::English:
        return "Anglais";
      case StringId::Spanish:
        return "Espagnol";
      case StringId::Portuguese:
        return "Portugais";
      case StringId::Russian:
        return "Russe";
      case StringId::French:
        return "Francais";
      case StringId::German:
        return "Allemand";
      case StringId::Italian:
        return "Italien";
      case StringId::Turkish:
        return "Turc";
      case StringId::Polish:
        return "Polonais";
      case StringId::Catalan:
        return "Catalan";
      case StringId::Basque:
        return "Basque";
      case StringId::Back:
        return "Retour";
      case StringId::On:
        return "Marche";
      case StringId::Off:
        return "Arret";
      case StringId::Enabled:
        return "Active";
      case StringId::Disabled:
        return "Desactive";
      case StringId::Start:
        return "Demarrer";
      case StringId::Stop:
        return "Arreter";
      case StringId::Pause:
        return "Pause";
      case StringId::Reset:
        return "Reinit.";
      case StringId::Set:
        return "Regler";
      case StringId::Charging:
        return "Charge";
      case StringId::FullyCharged:
        return "Chargee";
      case StringId::Discharging:
        return "Decharge";
      case StringId::BatteryLow:
        return "Batterie faible";
      case StringId::BatteryCritical:
        return "Batterie critique";
      case StringId::ReadingBattery:
        return "Lecture batterie";
      case StringId::Volts:
        return "volts";
      case StringId::DailyStepsGoal:
        return "Objectif jour";
      case StringId::Goal:
        return "Objectif";
      case StringId::Trip:
        return "Trajet";
      case StringId::Yesterday:
        return "Hier";
      case StringId::HeartRateBpm:
        return "BPM";
      case StringId::NotEnoughData:
        return "Donnees insuff.,\npatientez...";
      case StringId::NoTouchDetected:
        return "Aucun contact";
      case StringId::Measuring:
        return "Mesure...";
      case StringId::Stopped:
        return "Arrete";
      case StringId::BackgroundInterval:
        return "Interv. fond";
      case StringId::Continuous:
        return "Continu";
      case StringId::DisplayTimeout:
        return "Veille ecran";
      case StringId::AlwaysOn:
        return "Toujours actif";
      case StringId::SetCurrentTime:
        return "Regler heure";
      case StringId::SetCurrentDate:
        return "Regler date";
      case StringId::Metric:
        return "Metrique";
      case StringId::Imperial:
        return "Imperial";
      case StringId::EveryHour:
        return "Chaque heure";
      case StringId::Every30Minutes:
        return "Toutes les 30 min";
      case StringId::TillReboot:
        return "Jusqu'au redem.";
      case StringId::FirmwareAndFiles:
        return "Firmware et fichiers";
      case StringId::ClearSky:
        return "Ciel degage";
      case StringId::FewClouds:
        return "Peu nuageux";
      case StringId::ScatteredClouds:
        return "Nuages epars";
      case StringId::BrokenClouds:
        return "Tres nuageux";
      case StringId::ShowerRain:
        return "Averses";
      case StringId::Rain:
        return "Pluie";
      case StringId::Thunderstorm:
        return "Orage";
      case StringId::Snow:
        return "Neige";
      case StringId::Mist:
        return "Brouillard";
      case StringId::Clear:
        return "Clair";
      case StringId::Cloudy:
        return "Nuageux";
      case StringId::Showers:
        return "Averses";
      case StringId::Thunder:
        return "Tonnerre";
      case StringId::Notification:
        return "Notification";
      case StringId::IncomingCallFrom:
        return "Appel de";
      case StringId::Notifications:
        return "Notifications";
      case StringId::NoNotifications:
        return "Aucune notification";
      case StringId::TooLarge:
        return "Trop grand";
      case StringId::ZeroDivision:
        return "Division par zero";
      case StringId::Warning:
        return "Alerte";
      case StringId::TouchControllerError:
        return "Erreur tactile detectee.";
      case StringId::WarrantyTip:
        return "En cas de probleme sous garantie, contactez le vendeur.";
      case StringId::Proceed:
        return "Continuer";
      case StringId::Version:
        return "Version";
      case StringId::FirmwareValidated:
        return "Ce firmware a ete\n#00ff00 valide#";
      case StringId::FirmwareRollbackWarning:
        return "Tout redemarrage\nrestaurera le dernier\nfirmware valide";
      case StringId::Validate:
        return "Valider";
      case StringId::Rollback:
        return "Restaurer";
      case StringId::FirmwareUpdate:
        return "Mise a jour FW";
      case StringId::Waiting:
        return "Attente...";
      case StringId::WakeSensitivity:
        return "Sensib. reveil";
      case StringId::Calibrate:
        return "Calibrer";
      case StringId::Shake:
        return "Secouez!";
      case StringId::Ready:
        return "Pret!";
      case StringId::Connected:
        return "Connecte";
      case StringId::Disconnected:
        return "Deconnecte";
      case StringId::ImageOk:
        return "Image OK!";
      case StringId::Error:
        return "Erreur!";
      case StringId::YouHaveMail:
        return "Vous avez\ndu courrier!";
      case StringId::Navigation:
        return "Navigation";
      case StringId::Score:
        return "Score";
      case StringId::AlarmNotSet:
        return "Alarme\nnon\nreglee.";
      case StringId::TimeToAlarm:
        return "Temps avant\nalarme :";
      case StringId::Days:
        return "Jours";
      case StringId::Hours:
        return "Heures";
      case StringId::Minutes:
        return "Minutes";
      case StringId::Seconds:
        return "Secondes";
      case StringId::Once:
        return "Une fois";
      case StringId::Daily:
        return "Quotidien";
      case StringId::Weekdays:
        return "Lun-Ven";
      case StringId::Heads:
        return "Pile";
      case StringId::Tails:
        return "Face";
      default:
        return nullptr;
    }
  }

  const char* TranslateGerman(StringId id) {
    switch (id) {
      case StringId::Settings:
        return "Einstellungen";
      case StringId::Display:
        return "Anzeige";
      case StringId::WakeUp:
        return "Aufwecken";
      case StringId::TimeFormat:
        return "Zeitformat";
      case StringId::WatchFace:
        return "Zifferblatt";
      case StringId::Steps:
        return "Schritte";
      case StringId::HeartRate:
        return "Puls";
      case StringId::DateTime:
        return "Datum und Zeit";
      case StringId::Weather:
        return "Wetter";
      case StringId::Battery:
        return "Akku";
      case StringId::Chimes:
        return "Signale";
      case StringId::About:
        return "Info";
      case StringId::Language:
        return "Sprache";
      case StringId::English:
        return "Englisch";
      case StringId::Spanish:
        return "Spanisch";
      case StringId::Portuguese:
        return "Portugiesisch";
      case StringId::Russian:
        return "Russisch";
      case StringId::French:
        return "Franzosisch";
      case StringId::German:
        return "Deutsch";
      case StringId::Italian:
        return "Italienisch";
      case StringId::Turkish:
        return "Turkisch";
      case StringId::Polish:
        return "Polnisch";
      case StringId::Catalan:
        return "Katalanisch";
      case StringId::Basque:
        return "Baskisch";
      case StringId::Back:
        return "Zuruck";
      case StringId::On:
        return "An";
      case StringId::Off:
        return "Aus";
      case StringId::Enabled:
        return "Aktiv";
      case StringId::Disabled:
        return "Inaktiv";
      case StringId::Start:
        return "Start";
      case StringId::Stop:
        return "Stop";
      case StringId::Pause:
        return "Pause";
      case StringId::Reset:
        return "Reset";
      case StringId::Set:
        return "Setzen";
      case StringId::Charging:
        return "Laden";
      case StringId::FullyCharged:
        return "Voll geladen";
      case StringId::Discharging:
        return "Entladen";
      case StringId::BatteryLow:
        return "Akku schwach";
      case StringId::BatteryCritical:
        return "Akku kritisch";
      case StringId::ReadingBattery:
        return "Akku lesen";
      case StringId::Volts:
        return "Volt";
      case StringId::DailyStepsGoal:
        return "Tagesziel";
      case StringId::Goal:
        return "Ziel";
      case StringId::Trip:
        return "Strecke";
      case StringId::Yesterday:
        return "Gestern";
      case StringId::HeartRateBpm:
        return "Puls BPM";
      case StringId::NotEnoughData:
        return "Zu wenig Daten,\nbitte warten...";
      case StringId::NoTouchDetected:
        return "Kein Kontakt";
      case StringId::Measuring:
        return "Messe...";
      case StringId::Stopped:
        return "Gestoppt";
      case StringId::BackgroundInterval:
        return "Hintergr. Interv.";
      case StringId::Continuous:
        return "Dauernd";
      case StringId::DisplayTimeout:
        return "Bildschirmzeit";
      case StringId::AlwaysOn:
        return "Immer an";
      case StringId::SetCurrentTime:
        return "Zeit setzen";
      case StringId::SetCurrentDate:
        return "Datum setzen";
      case StringId::Metric:
        return "Metrisch";
      case StringId::Imperial:
        return "Imperial";
      case StringId::EveryHour:
        return "Jede Stunde";
      case StringId::Every30Minutes:
        return "Alle 30 Min";
      case StringId::TillReboot:
        return "Bis Neustart";
      case StringId::FirmwareAndFiles:
        return "Firmware und Dateien";
      case StringId::ClearSky:
        return "Klarer Himmel";
      case StringId::FewClouds:
        return "Wenige Wolken";
      case StringId::ScatteredClouds:
        return "Aufgelockert";
      case StringId::BrokenClouds:
        return "Stark bewolkt";
      case StringId::ShowerRain:
        return "Schauer";
      case StringId::Rain:
        return "Regen";
      case StringId::Thunderstorm:
        return "Gewitter";
      case StringId::Snow:
        return "Schnee";
      case StringId::Mist:
        return "Nebel";
      case StringId::Clear:
        return "Klar";
      case StringId::Cloudy:
        return "Wolkig";
      case StringId::Showers:
        return "Schauer";
      case StringId::Thunder:
        return "Donner";
      case StringId::Notification:
        return "Benachrichtigung";
      case StringId::IncomingCallFrom:
        return "Anruf von";
      case StringId::Notifications:
        return "Benachrichtigungen";
      case StringId::NoNotifications:
        return "Keine Benachrichtigungen";
      case StringId::TooLarge:
        return "Zu gross";
      case StringId::ZeroDivision:
        return "Division durch null";
      case StringId::Warning:
        return "Warnung";
      case StringId::TouchControllerError:
        return "Touch-Fehler erkannt.";
      case StringId::WarrantyTip:
        return "Bei Problemen in der Garantie den Verkaufer kontaktieren.";
      case StringId::Proceed:
        return "Fortfahren";
      case StringId::Version:
        return "Version";
      case StringId::FirmwareValidated:
        return "Diese Firmware ist\n#00ff00 validiert#";
      case StringId::FirmwareRollbackWarning:
        return "Jeder Neustart\nsetzt die letzte\nvalidierte Firmware zuruck";
      case StringId::Validate:
        return "Validieren";
      case StringId::Rollback:
        return "Zuruck";
      case StringId::FirmwareUpdate:
        return "Firmware-Update";
      case StringId::Waiting:
        return "Warten...";
      case StringId::WakeSensitivity:
        return "Weck-Sensib.";
      case StringId::Calibrate:
        return "Kalibrieren";
      case StringId::Shake:
        return "Schutteln!";
      case StringId::Ready:
        return "Bereit!";
      case StringId::Connected:
        return "Verbunden";
      case StringId::Disconnected:
        return "Getrennt";
      case StringId::ImageOk:
        return "Image OK!";
      case StringId::Error:
        return "Fehler!";
      case StringId::YouHaveMail:
        return "Sie haben\nPost!";
      case StringId::Navigation:
        return "Navigation";
      case StringId::Score:
        return "Punkte";
      case StringId::AlarmNotSet:
        return "Alarm\nist nicht\ngesetzt.";
      case StringId::TimeToAlarm:
        return "Zeit bis\nAlarm:";
      case StringId::Days:
        return "Tage";
      case StringId::Hours:
        return "Stunden";
      case StringId::Minutes:
        return "Minuten";
      case StringId::Seconds:
        return "Sekunden";
      case StringId::Once:
        return "Einmal";
      case StringId::Daily:
        return "Taglich";
      case StringId::Weekdays:
        return "Mo-Fr";
      case StringId::Heads:
        return "Kopf";
      case StringId::Tails:
        return "Zahl";
      default:
        return nullptr;
    }
  }

  const char* TranslateItalian(StringId id) {
    switch (id) {
      case StringId::Settings:
        return "Impostazioni";
      case StringId::Display:
        return "Schermo";
      case StringId::WakeUp:
        return "Risveglio";
      case StringId::TimeFormat:
        return "Formato ora";
      case StringId::WatchFace:
        return "Quadrante";
      case StringId::Steps:
        return "Passi";
      case StringId::HeartRate:
        return "Battito";
      case StringId::DateTime:
        return "Data e ora";
      case StringId::Weather:
        return "Meteo";
      case StringId::Battery:
        return "Batteria";
      case StringId::Chimes:
        return "Rintocchi";
      case StringId::About:
        return "Info";
      case StringId::Language:
        return "Lingua";
      case StringId::English:
        return "Inglese";
      case StringId::Spanish:
        return "Spagnolo";
      case StringId::Portuguese:
        return "Portoghese";
      case StringId::Russian:
        return "Russo";
      case StringId::French:
        return "Francese";
      case StringId::German:
        return "Tedesco";
      case StringId::Italian:
        return "Italiano";
      case StringId::Turkish:
        return "Turco";
      case StringId::Polish:
        return "Polacco";
      case StringId::Catalan:
        return "Catalano";
      case StringId::Basque:
        return "Basco";
      case StringId::Back:
        return "Indietro";
      case StringId::On:
        return "Acc.";
      case StringId::Off:
        return "Spento";
      case StringId::Enabled:
        return "Attivo";
      case StringId::Disabled:
        return "Disattivo";
      case StringId::Start:
        return "Avvia";
      case StringId::Stop:
        return "Ferma";
      case StringId::Pause:
        return "Pausa";
      case StringId::Reset:
        return "Azzera";
      case StringId::Set:
        return "Imposta";
      case StringId::Charging:
        return "In carica";
      case StringId::FullyCharged:
        return "Carica completa";
      case StringId::Discharging:
        return "Scarica";
      case StringId::BatteryLow:
        return "Batteria bassa";
      case StringId::BatteryCritical:
        return "Batteria critica";
      case StringId::ReadingBattery:
        return "Lettura batteria";
      case StringId::Volts:
        return "volt";
      case StringId::DailyStepsGoal:
        return "Obiettivo giorn.";
      case StringId::Goal:
        return "Obiettivo";
      case StringId::Trip:
        return "Tragitto";
      case StringId::Yesterday:
        return "Ieri";
      case StringId::HeartRateBpm:
        return "Battiti/min";
      case StringId::NotEnoughData:
        return "Dati insufficienti,\nattendi...";
      case StringId::NoTouchDetected:
        return "Nessun contatto";
      case StringId::Measuring:
        return "Misura...";
      case StringId::Stopped:
        return "Fermato";
      case StringId::BackgroundInterval:
        return "Intervallo bg";
      case StringId::Continuous:
        return "Continuo";
      case StringId::DisplayTimeout:
        return "Timeout schermo";
      case StringId::AlwaysOn:
        return "Sempre acceso";
      case StringId::SetCurrentTime:
        return "Imposta ora";
      case StringId::SetCurrentDate:
        return "Imposta data";
      case StringId::Metric:
        return "Metrico";
      case StringId::Imperial:
        return "Imperiale";
      case StringId::EveryHour:
        return "Ogni ora";
      case StringId::Every30Minutes:
        return "Ogni 30 min";
      case StringId::TillReboot:
        return "Fino al riavvio";
      case StringId::FirmwareAndFiles:
        return "Firmware e file";
      case StringId::ClearSky:
        return "Sereno";
      case StringId::FewClouds:
        return "Poche nuvole";
      case StringId::ScatteredClouds:
        return "Nuvole sparse";
      case StringId::BrokenClouds:
        return "Molto nuvoloso";
      case StringId::ShowerRain:
        return "Rovesci";
      case StringId::Rain:
        return "Pioggia";
      case StringId::Thunderstorm:
        return "Temporale";
      case StringId::Snow:
        return "Neve";
      case StringId::Mist:
        return "Nebbia";
      case StringId::Clear:
        return "Sereno";
      case StringId::Cloudy:
        return "Nuvoloso";
      case StringId::Showers:
        return "Rovesci";
      case StringId::Thunder:
        return "Tuono";
      case StringId::Notification:
        return "Notifica";
      case StringId::IncomingCallFrom:
        return "Chiamata da";
      case StringId::Notifications:
        return "Notifiche";
      case StringId::NoNotifications:
        return "Nessuna notifica";
      case StringId::TooLarge:
        return "Troppo grande";
      case StringId::ZeroDivision:
        return "Divisione per zero";
      case StringId::Warning:
        return "Avviso";
      case StringId::TouchControllerError:
        return "Errore touch rilevato.";
      case StringId::WarrantyTip:
        return "Se ci sono problemi in garanzia, contatta il venditore.";
      case StringId::Proceed:
        return "Continua";
      case StringId::Version:
        return "Versione";
      case StringId::FirmwareValidated:
        return "Questo firmware e\n#00ff00 validato#";
      case StringId::FirmwareRollbackWarning:
        return "Qualsiasi riavvio\nripristinera l'ultimo\nfirmware validato";
      case StringId::Validate:
        return "Valida";
      case StringId::Rollback:
        return "Ripristina";
      case StringId::FirmwareUpdate:
        return "Agg. firmware";
      case StringId::Waiting:
        return "Attesa...";
      case StringId::WakeSensitivity:
        return "Sensib. risveglio";
      case StringId::Calibrate:
        return "Calibra";
      case StringId::Shake:
        return "Scuoti!";
      case StringId::Ready:
        return "Pronto!";
      case StringId::Connected:
        return "Connesso";
      case StringId::Disconnected:
        return "Disconnesso";
      case StringId::ImageOk:
        return "Immagine OK!";
      case StringId::Error:
        return "Errore!";
      case StringId::YouHaveMail:
        return "Hai\nposta!";
      case StringId::Navigation:
        return "Navigazione";
      case StringId::Score:
        return "Punteggio";
      case StringId::AlarmNotSet:
        return "Sveglia\nnon\nimpostata.";
      case StringId::TimeToAlarm:
        return "Tempo alla\nsveglia:";
      case StringId::Days:
        return "Giorni";
      case StringId::Hours:
        return "Ore";
      case StringId::Minutes:
        return "Minuti";
      case StringId::Seconds:
        return "Secondi";
      case StringId::Once:
        return "Una volta";
      case StringId::Daily:
        return "Ogni giorno";
      case StringId::Weekdays:
        return "Lun-Ven";
      case StringId::Heads:
        return "Testa";
      case StringId::Tails:
        return "Croce";
      default:
        return nullptr;
    }
  }

  const char* TranslateTurkish(StringId id) {
    switch (id) {
      case StringId::Settings:
        return "Ayarlar";
      case StringId::Display:
        return "Ekran";
      case StringId::WakeUp:
        return "Uyanma";
      case StringId::TimeFormat:
        return "Saat bicimi";
      case StringId::WatchFace:
        return "Saat yuzu";
      case StringId::Steps:
        return "Adimlar";
      case StringId::HeartRate:
        return "Nabiz";
      case StringId::DateTime:
        return "Tarih ve saat";
      case StringId::Weather:
        return "Hava";
      case StringId::Battery:
        return "Pil";
      case StringId::Chimes:
        return "Saat uyarisi";
      case StringId::About:
        return "Hakkinda";
      case StringId::Language:
        return "Dil";
      case StringId::English:
        return "Ingilizce";
      case StringId::Spanish:
        return "Ispanyolca";
      case StringId::Portuguese:
        return "Portekizce";
      case StringId::Russian:
        return "Rusca";
      case StringId::French:
        return "Fransizca";
      case StringId::German:
        return "Almanca";
      case StringId::Italian:
        return "Italyanca";
      case StringId::Turkish:
        return "Turkce";
      case StringId::Polish:
        return "Lehce";
      case StringId::Catalan:
        return "Katalanca";
      case StringId::Basque:
        return "Baskca";
      case StringId::Back:
        return "Geri";
      case StringId::On:
        return "Acik";
      case StringId::Off:
        return "Kapali";
      case StringId::Enabled:
        return "Etkin";
      case StringId::Disabled:
        return "Devre disi";
      case StringId::Start:
        return "Baslat";
      case StringId::Stop:
        return "Durdur";
      case StringId::Pause:
        return "Duraklat";
      case StringId::Reset:
        return "Sifirla";
      case StringId::Set:
        return "Ayarla";
      case StringId::Charging:
        return "Sarj oluyor";
      case StringId::FullyCharged:
        return "Tam dolu";
      case StringId::Discharging:
        return "Desarj oluyor";
      case StringId::BatteryLow:
        return "Pil dusuk";
      case StringId::BatteryCritical:
        return "Pil kritik";
      case StringId::ReadingBattery:
        return "Pil okunuyor";
      case StringId::Volts:
        return "volt";
      case StringId::DailyStepsGoal:
        return "Gunluk hedef";
      case StringId::Goal:
        return "Hedef";
      case StringId::Trip:
        return "Tur";
      case StringId::Yesterday:
        return "Dun";
      case StringId::HeartRateBpm:
        return "Nabiz BPM";
      case StringId::NotEnoughData:
        return "Yetersiz veri,\nlutfen bekleyin...";
      case StringId::NoTouchDetected:
        return "Dokunus yok";
      case StringId::Measuring:
        return "Olculuyor...";
      case StringId::Stopped:
        return "Durdu";
      case StringId::BackgroundInterval:
        return "Arka plan araligi";
      case StringId::Continuous:
        return "Surekli";
      case StringId::DisplayTimeout:
        return "Ekran suresi";
      case StringId::AlwaysOn:
        return "Hep acik";
      case StringId::SetCurrentTime:
        return "Saati ayarla";
      case StringId::SetCurrentDate:
        return "Tarihi ayarla";
      case StringId::Metric:
        return "Metrik";
      case StringId::Imperial:
        return "Imperyal";
      case StringId::EveryHour:
        return "Her saat";
      case StringId::Every30Minutes:
        return "Her 30 dk";
      case StringId::TillReboot:
        return "Yeniden baslatana dek";
      case StringId::FirmwareAndFiles:
        return "Firmware ve dosyalar";
      case StringId::ClearSky:
        return "Acik";
      case StringId::FewClouds:
        return "Az bulutlu";
      case StringId::ScatteredClouds:
        return "Parcali bulutlu";
      case StringId::BrokenClouds:
        return "Cok bulutlu";
      case StringId::ShowerRain:
        return "Saganak";
      case StringId::Rain:
        return "Yagmur";
      case StringId::Thunderstorm:
        return "Firtina";
      case StringId::Snow:
        return "Kar";
      case StringId::Mist:
        return "Sis";
      case StringId::Clear:
        return "Acik";
      case StringId::Cloudy:
        return "Bulutlu";
      case StringId::Showers:
        return "Saganak";
      case StringId::Thunder:
        return "Gok gurultusu";
      case StringId::Notification:
        return "Bildirim";
      case StringId::IncomingCallFrom:
        return "Arayan";
      case StringId::Notifications:
        return "Bildirimler";
      case StringId::NoNotifications:
        return "Bildirim yok";
      case StringId::TooLarge:
        return "Cok buyuk";
      case StringId::ZeroDivision:
        return "Sifira bolme";
      case StringId::Warning:
        return "Uyari";
      case StringId::TouchControllerError:
        return "Dokunmatik hatasi algilandi.";
      case StringId::WarrantyTip:
        return "Sorun olursa ve garanti varsa saticiyla iletisime gecin.";
      case StringId::Proceed:
        return "Devam";
      case StringId::Version:
        return "Surum";
      case StringId::ShortRef:
        return "Kisa ref";
      case StringId::FirmwareValidated:
        return "Bu firmware\n#00ff00 dogrulandi#";
      case StringId::FirmwareRollbackWarning:
        return "Her yeniden baslatma\nson dogrulanan\nfirmware'e doner";
      case StringId::Validate:
        return "Dogrula";
      case StringId::Rollback:
        return "Geri al";
      case StringId::FirmwareUpdate:
        return "Firmware guncelleme";
      case StringId::Waiting:
        return "Bekleniyor...";
      case StringId::WakeSensitivity:
        return "Uyanma hass.";
      case StringId::Calibrate:
        return "Kalibre et";
      case StringId::Shake:
        return "Salla!";
      case StringId::Ready:
        return "Hazir!";
      case StringId::Connected:
        return "Bagli";
      case StringId::Disconnected:
        return "Bagli degil";
      case StringId::ImageOk:
        return "Imge tamam!";
      case StringId::Error:
        return "Hata!";
      case StringId::YouHaveMail:
        return "Yeni\nmesajin var!";
      case StringId::Navigation:
        return "Navigasyon";
      case StringId::Score:
        return "Skor";
      case StringId::AlarmNotSet:
        return "Alarm\nkurulu\ndegil.";
      case StringId::TimeToAlarm:
        return "Alarma\nkalan:";
      case StringId::Days:
        return "Gun";
      case StringId::Hours:
        return "Saat";
      case StringId::Minutes:
        return "Dakika";
      case StringId::Seconds:
        return "Saniye";
      case StringId::Once:
        return "Bir kez";
      case StringId::Daily:
        return "Her gun";
      case StringId::Weekdays:
        return "Pzt-Cum";
      case StringId::Heads:
        return "Yazi";
      case StringId::Tails:
        return "Tura";
      default:
        return nullptr;
    }
  }

  const char* TranslatePolish(StringId id) {
    switch (id) {
      case StringId::Settings:
        return "Ustawienia";
      case StringId::Display:
        return "Ekran";
      case StringId::WakeUp:
        return "Wybudzanie";
      case StringId::TimeFormat:
        return "Format czasu";
      case StringId::WatchFace:
        return "Tarcza";
      case StringId::Steps:
        return "Kroki";
      case StringId::HeartRate:
        return "Tetno";
      case StringId::DateTime:
        return "Data i czas";
      case StringId::Weather:
        return "Pogoda";
      case StringId::Battery:
        return "Bateria";
      case StringId::Chimes:
        return "Sygnaly";
      case StringId::About:
        return "O zegarku";
      case StringId::Language:
        return "Jezyk";
      case StringId::English:
        return "Angielski";
      case StringId::Spanish:
        return "Hiszpanski";
      case StringId::Portuguese:
        return "Portugalski";
      case StringId::Russian:
        return "Rosyjski";
      case StringId::French:
        return "Francuski";
      case StringId::German:
        return "Niemiecki";
      case StringId::Italian:
        return "Wloski";
      case StringId::Turkish:
        return "Turecki";
      case StringId::Polish:
        return "Polski";
      case StringId::Catalan:
        return "Katalonski";
      case StringId::Basque:
        return "Baskijski";
      case StringId::Back:
        return "Wstecz";
      case StringId::On:
        return "Wl.";
      case StringId::Off:
        return "Wyl.";
      case StringId::Enabled:
        return "Wlaczone";
      case StringId::Disabled:
        return "Wylaczone";
      case StringId::Start:
        return "Start";
      case StringId::Stop:
        return "Stop";
      case StringId::Pause:
        return "Pauza";
      case StringId::Reset:
        return "Reset";
      case StringId::Set:
        return "Ustaw";
      case StringId::Charging:
        return "Ladowanie";
      case StringId::FullyCharged:
        return "Naladowana";
      case StringId::Discharging:
        return "Rozladowanie";
      case StringId::BatteryLow:
        return "Niski poziom";
      case StringId::BatteryCritical:
        return "Krytyczny poziom";
      case StringId::ReadingBattery:
        return "Odczyt baterii";
      case StringId::Volts:
        return "V";
      case StringId::DailyStepsGoal:
        return "Cel dzienny";
      case StringId::Goal:
        return "Cel";
      case StringId::Trip:
        return "Sesja";
      case StringId::Yesterday:
        return "Wczoraj";
      case StringId::HeartRateBpm:
        return "Tetno BPM";
      case StringId::NotEnoughData:
        return "Za malo danych,\nczekaj...";
      case StringId::NoTouchDetected:
        return "Brak dotyku";
      case StringId::Measuring:
        return "Pomiar...";
      case StringId::Stopped:
        return "Zatrzymano";
      case StringId::BackgroundInterval:
        return "Interwal tla";
      case StringId::Continuous:
        return "Ciagle";
      case StringId::DisplayTimeout:
        return "Wygaszanie";
      case StringId::AlwaysOn:
        return "Zawsze wl.";
      case StringId::SetCurrentTime:
        return "Ustaw czas";
      case StringId::SetCurrentDate:
        return "Ustaw date";
      case StringId::Metric:
        return "Metryczne";
      case StringId::Imperial:
        return "Imperialne";
      case StringId::EveryHour:
        return "Co godzine";
      case StringId::Every30Minutes:
        return "Co 30 min";
      case StringId::TillReboot:
        return "Do restartu";
      case StringId::FirmwareAndFiles:
        return "Firmware i pliki";
      case StringId::ClearSky:
        return "Bezchmurnie";
      case StringId::FewClouds:
        return "Malo chmur";
      case StringId::ScatteredClouds:
        return "Chmury";
      case StringId::BrokenClouds:
        return "Pochmurno";
      case StringId::ShowerRain:
        return "Przelotnie";
      case StringId::Rain:
        return "Deszcz";
      case StringId::Thunderstorm:
        return "Burza";
      case StringId::Snow:
        return "Snieg";
      case StringId::Mist:
        return "Mgla";
      case StringId::Clear:
        return "Pogodnie";
      case StringId::Cloudy:
        return "Pochmurno";
      case StringId::Showers:
        return "Przelotnie";
      case StringId::Thunder:
        return "Grzmot";
      case StringId::Notification:
        return "Powiadomienie";
      case StringId::IncomingCallFrom:
        return "Polaczenie od";
      case StringId::Notifications:
        return "Powiadomienia";
      case StringId::NoNotifications:
        return "Brak powiadomien";
      case StringId::TooLarge:
        return "Za duze";
      case StringId::ZeroDivision:
        return "Dzielenie przez zero";
      case StringId::Warning:
        return "Ostrzezenie";
      case StringId::TouchControllerError:
        return "Wykryto blad dotyku.";
      case StringId::WarrantyTip:
        return "Jesli sa problemy i jest gwarancja, skontaktuj sie ze sprzedawca.";
      case StringId::Proceed:
        return "Dalej";
      case StringId::Version:
        return "Wersja";
      case StringId::ShortRef:
        return "Krotki ref";
      case StringId::FirmwareValidated:
        return "To firmware jest\n#00ff00 zweryfikowane#";
      case StringId::FirmwareRollbackWarning:
        return "Kazdy restart\nprzywroci ostatnie\nzweryfikowane firmware";
      case StringId::Validate:
        return "Zweryfikuj";
      case StringId::Rollback:
        return "Cofnij";
      case StringId::FirmwareUpdate:
        return "Aktualizacja FW";
      case StringId::Waiting:
        return "Oczekiwanie...";
      case StringId::WakeSensitivity:
        return "Czulosc budz.";
      case StringId::Calibrate:
        return "Kalibruj";
      case StringId::Shake:
        return "Potrzasnij!";
      case StringId::Ready:
        return "Gotowe!";
      case StringId::Connected:
        return "Polaczono";
      case StringId::Disconnected:
        return "Rozlaczono";
      case StringId::ImageOk:
        return "Obraz OK!";
      case StringId::Error:
        return "Blad!";
      case StringId::YouHaveMail:
        return "Masz\nwiadomosc!";
      case StringId::Navigation:
        return "Nawigacja";
      case StringId::Score:
        return "Wynik";
      case StringId::AlarmNotSet:
        return "Alarm\nnie jest\nustawiony.";
      case StringId::TimeToAlarm:
        return "Do alarmu:";
      case StringId::Days:
        return "Dni";
      case StringId::Hours:
        return "Godzin";
      case StringId::Minutes:
        return "Minut";
      case StringId::Seconds:
        return "Sekund";
      case StringId::Once:
        return "Raz";
      case StringId::Daily:
        return "Codziennie";
      case StringId::Weekdays:
        return "Pon-Pt";
      case StringId::Heads:
        return "Orzel";
      case StringId::Tails:
        return "Reszka";
      default:
        return nullptr;
    }
  }

  const char* TranslateCatalan(StringId id) {
    switch (id) {
      case StringId::Settings:
        return "Ajustos";
      case StringId::Display:
        return "Pantalla";
      case StringId::WakeUp:
        return "Activacio";
      case StringId::TimeFormat:
        return "Format hora";
      case StringId::WatchFace:
        return "Esfera";
      case StringId::Steps:
        return "Passos";
      case StringId::HeartRate:
        return "Pols";
      case StringId::DateTime:
        return "Data i hora";
      case StringId::Weather:
        return "Temps";
      case StringId::Battery:
        return "Bateria";
      case StringId::Chimes:
        return "Campanades";
      case StringId::About:
        return "Quant a";
      case StringId::Language:
        return "Idioma";
      case StringId::English:
        return "Angles";
      case StringId::Spanish:
        return "Castella";
      case StringId::Portuguese:
        return "Portugues";
      case StringId::Russian:
        return "Rus";
      case StringId::French:
        return "Frances";
      case StringId::German:
        return "Alemany";
      case StringId::Italian:
        return "Italia";
      case StringId::Turkish:
        return "Turc";
      case StringId::Polish:
        return "Polones";
      case StringId::Catalan:
        return "Catala";
      case StringId::Basque:
        return "Euskara";
      case StringId::Back:
        return "Enrere";
      case StringId::On:
        return "Enc.";
      case StringId::Off:
        return "Apag.";
      case StringId::Enabled:
        return "Activat";
      case StringId::Disabled:
        return "Desactivat";
      case StringId::Start:
        return "Inicia";
      case StringId::Stop:
        return "Atura";
      case StringId::Pause:
        return "Pausa";
      case StringId::Reset:
        return "Reinicia";
      case StringId::Set:
        return "Fixa";
      case StringId::Charging:
        return "Carregant";
      case StringId::FullyCharged:
        return "Carrega completa";
      case StringId::Discharging:
        return "Descarregant";
      case StringId::BatteryLow:
        return "Bateria baixa";
      case StringId::BatteryCritical:
        return "Bateria critica";
      case StringId::ReadingBattery:
        return "Llegint bateria";
      case StringId::Volts:
        return "volts";
      case StringId::DailyStepsGoal:
        return "Objectiu diari";
      case StringId::Goal:
        return "Objectiu";
      case StringId::Trip:
        return "Trajecte";
      case StringId::Yesterday:
        return "Ahir";
      case StringId::HeartRateBpm:
        return "Pols BPM";
      case StringId::NotEnoughData:
        return "Dades insuf.,\nespera...";
      case StringId::NoTouchDetected:
        return "Sense contacte";
      case StringId::Measuring:
        return "Mesurant...";
      case StringId::Stopped:
        return "Aturat";
      case StringId::BackgroundInterval:
        return "Interval fons";
      case StringId::Continuous:
        return "Continu";
      case StringId::DisplayTimeout:
        return "Temps pantalla";
      case StringId::AlwaysOn:
        return "Sempre activa";
      case StringId::SetCurrentTime:
        return "Ajusta hora";
      case StringId::SetCurrentDate:
        return "Ajusta data";
      case StringId::Metric:
        return "Metric";
      case StringId::Imperial:
        return "Imperial";
      case StringId::EveryHour:
        return "Cada hora";
      case StringId::Every30Minutes:
        return "Cada 30 min";
      case StringId::TillReboot:
        return "Fins reinici";
      case StringId::FirmwareAndFiles:
        return "Firmware i fitxers";
      case StringId::ClearSky:
        return "Cel clar";
      case StringId::FewClouds:
        return "Pocs nuvols";
      case StringId::ScatteredClouds:
        return "Nuvols dispersos";
      case StringId::BrokenClouds:
        return "Molt ennuvolat";
      case StringId::ShowerRain:
        return "Ruixats";
      case StringId::Rain:
        return "Pluja";
      case StringId::Thunderstorm:
        return "Tempesta";
      case StringId::Snow:
        return "Neu";
      case StringId::Mist:
        return "Boira";
      case StringId::Clear:
        return "Clar";
      case StringId::Cloudy:
        return "Ennuvolat";
      case StringId::Showers:
        return "Ruixats";
      case StringId::Thunder:
        return "Tro";
      case StringId::Notification:
        return "Notificacio";
      case StringId::IncomingCallFrom:
        return "Trucada de";
      case StringId::Notifications:
        return "Notificacions";
      case StringId::NoNotifications:
        return "Sense notificacions";
      case StringId::TooLarge:
        return "Massa gran";
      case StringId::ZeroDivision:
        return "Divisio per zero";
      case StringId::Warning:
        return "Avis";
      case StringId::TouchControllerError:
        return "Error tactil detectat.";
      case StringId::WarrantyTip:
        return "Si hi ha problemes i es en garantia, contacta amb el venedor.";
      case StringId::Proceed:
        return "Continua";
      case StringId::Version:
        return "Versio";
      case StringId::ShortRef:
        return "Ref curta";
      case StringId::FirmwareValidated:
        return "Aquest firmware ha\nestat #00ff00 validat#";
      case StringId::FirmwareRollbackWarning:
        return "Qualsevol reinici\nrestaurara l'ultim\nfirmware validat";
      case StringId::Validate:
        return "Valida";
      case StringId::Rollback:
        return "Reverteix";
      case StringId::FirmwareUpdate:
        return "Actual. firmware";
      case StringId::Waiting:
        return "Esperant...";
      case StringId::WakeSensitivity:
        return "Sensib. despert.";
      case StringId::Calibrate:
        return "Calibra";
      case StringId::Shake:
        return "Sacseja!";
      case StringId::Ready:
        return "Llest!";
      case StringId::Connected:
        return "Connectat";
      case StringId::Disconnected:
        return "Desconnectat";
      case StringId::ImageOk:
        return "Imatge OK!";
      case StringId::Error:
        return "Error!";
      case StringId::YouHaveMail:
        return "Tens\ncorreu!";
      case StringId::Navigation:
        return "Navegacio";
      case StringId::Score:
        return "Puntuacio";
      case StringId::AlarmNotSet:
        return "L'alarma\nno esta\nfixada.";
      case StringId::TimeToAlarm:
        return "Temps fins\na l'alarma:";
      case StringId::Days:
        return "Dies";
      case StringId::Hours:
        return "Hores";
      case StringId::Minutes:
        return "Minuts";
      case StringId::Seconds:
        return "Segons";
      case StringId::Once:
        return "Un cop";
      case StringId::Daily:
        return "Cada dia";
      case StringId::Weekdays:
        return "Dl-Dv";
      case StringId::Heads:
        return "Cara";
      case StringId::Tails:
        return "Creu";
      default:
        return nullptr;
    }
  }

  const char* TranslateBasque(StringId id) {
    switch (id) {
      case StringId::Settings:
        return "Ezarpenak";
      case StringId::Display:
        return "Pantaila";
      case StringId::WakeUp:
        return "Esnatu";
      case StringId::TimeFormat:
        return "Ordu formatua";
      case StringId::WatchFace:
        return "Erloju aurpegia";
      case StringId::Steps:
        return "Urratsak";
      case StringId::HeartRate:
        return "Taupadak";
      case StringId::DateTime:
        return "Data eta ordua";
      case StringId::Weather:
        return "Eguraldia";
      case StringId::Battery:
        return "Bateria";
      case StringId::Chimes:
        return "Txintak";
      case StringId::About:
        return "Honi buruz";
      case StringId::Language:
        return "Hizkuntza";
      case StringId::English:
        return "Ingelesa";
      case StringId::Spanish:
        return "Gaztelania";
      case StringId::Portuguese:
        return "Portugesa";
      case StringId::Russian:
        return "Errusiera";
      case StringId::French:
        return "Frantsesa";
      case StringId::German:
        return "Alemana";
      case StringId::Italian:
        return "Italiera";
      case StringId::Turkish:
        return "Turkiera";
      case StringId::Polish:
        return "Poloniera";
      case StringId::Catalan:
        return "Katalana";
      case StringId::Basque:
        return "Euskara";
      case StringId::Back:
        return "Atzera";
      case StringId::On:
        return "Piztuta";
      case StringId::Off:
        return "Itzalita";
      case StringId::Enabled:
        return "Gaituta";
      case StringId::Disabled:
        return "Desgaituta";
      case StringId::Start:
        return "Hasi";
      case StringId::Stop:
        return "Gelditu";
      case StringId::Pause:
        return "Pausatu";
      case StringId::Reset:
        return "Berrezarri";
      case StringId::Set:
        return "Ezarri";
      case StringId::Charging:
        return "Kargatzen";
      case StringId::FullyCharged:
        return "Osoa kargatuta";
      case StringId::Discharging:
        return "Deskargatzen";
      case StringId::BatteryLow:
        return "Bateria baxua";
      case StringId::BatteryCritical:
        return "Bateria kritikoa";
      case StringId::ReadingBattery:
        return "Bateria irakurtzen";
      case StringId::Volts:
        return "volt";
      case StringId::DailyStepsGoal:
        return "Eguneko helburua";
      case StringId::Goal:
        return "Helburua";
      case StringId::Trip:
        return "Saioa";
      case StringId::Yesterday:
        return "Atzo";
      case StringId::HeartRateBpm:
        return "Taupadak/min";
      case StringId::NotEnoughData:
        return "Datu gutxiegi,\nitxaron...";
      case StringId::NoTouchDetected:
        return "Ez da ukitu";
      case StringId::Measuring:
        return "Neurtzen...";
      case StringId::Stopped:
        return "Geldituta";
      case StringId::BackgroundInterval:
        return "Atzeko tartea";
      case StringId::Continuous:
        return "Etengabe";
      case StringId::DisplayTimeout:
        return "Pantaila denbora";
      case StringId::AlwaysOn:
        return "Beti piztuta";
      case StringId::SetCurrentTime:
        return "Ordua ezarri";
      case StringId::SetCurrentDate:
        return "Data ezarri";
      case StringId::Metric:
        return "Metrikoa";
      case StringId::Imperial:
        return "Imperiala";
      case StringId::EveryHour:
        return "Orduro";
      case StringId::Every30Minutes:
        return "30 minuturo";
      case StringId::TillReboot:
        return "Berrabiarazi arte";
      case StringId::FirmwareAndFiles:
        return "Firmware eta fitxategiak";
      case StringId::ClearSky:
        return "Zeru garbia";
      case StringId::FewClouds:
        return "Hodei gutxi";
      case StringId::ScatteredClouds:
        return "Hodei sakabanatuak";
      case StringId::BrokenClouds:
        return "Oso hodeitsu";
      case StringId::ShowerRain:
        return "Zaparradak";
      case StringId::Rain:
        return "Euria";
      case StringId::Thunderstorm:
        return "Ekaitza";
      case StringId::Snow:
        return "Elurra";
      case StringId::Mist:
        return "Lainoa";
      case StringId::Clear:
        return "Garbi";
      case StringId::Cloudy:
        return "Hodeitsu";
      case StringId::Showers:
        return "Zaparradak";
      case StringId::Thunder:
        return "Trumoia";
      case StringId::Notification:
        return "Jakinarazpena";
      case StringId::IncomingCallFrom:
        return "Deia";
      case StringId::Notifications:
        return "Jakinarazpenak";
      case StringId::NoNotifications:
        return "Ez dago jakinarazpenik";
      case StringId::TooLarge:
        return "Handiegia";
      case StringId::ZeroDivision:
        return "Zeroz zatitzea";
      case StringId::Warning:
        return "Abisua";
      case StringId::TouchControllerError:
        return "Ukimen errorea atzeman da.";
      case StringId::WarrantyTip:
        return "Arazorik badago eta bermea badu, jarri harremanetan saltzailearekin.";
      case StringId::Proceed:
        return "Jarraitu";
      case StringId::Version:
        return "Bertsioa";
      case StringId::ShortRef:
        return "Erref laburra";
      case StringId::FirmwareValidated:
        return "Firmware hau\n#00ff00 balioztatu da#";
      case StringId::FirmwareRollbackWarning:
        return "Edozein berrabiaraztek\nazken firmware\nbalioztatua itzuliko du";
      case StringId::Validate:
        return "Balioztatu";
      case StringId::Rollback:
        return "Leheneratu";
      case StringId::FirmwareUpdate:
        return "Firmware eguner.";
      case StringId::Waiting:
        return "Itxaroten...";
      case StringId::WakeSensitivity:
        return "Esnatze sent.";
      case StringId::Calibrate:
        return "Kalibratu";
      case StringId::Shake:
        return "Astindu!";
      case StringId::Ready:
        return "Prest!";
      case StringId::Connected:
        return "Konektatuta";
      case StringId::Disconnected:
        return "Deskonektatuta";
      case StringId::ImageOk:
        return "Irudia ondo!";
      case StringId::Error:
        return "Errorea!";
      case StringId::YouHaveMail:
        return "Posta\nberria duzu!";
      case StringId::Navigation:
        return "Nabigazioa";
      case StringId::Score:
        return "Puntuak";
      case StringId::AlarmNotSet:
        return "Alarma\nez dago\njarrita.";
      case StringId::TimeToAlarm:
        return "Alarmarako\ngeratzen da:";
      case StringId::Days:
        return "Egun";
      case StringId::Hours:
        return "Ordu";
      case StringId::Minutes:
        return "Minutu";
      case StringId::Seconds:
        return "Segundo";
      case StringId::Once:
        return "Behin";
      case StringId::Daily:
        return "Egunero";
      case StringId::Weekdays:
        return "Al-Or";
      case StringId::Heads:
        return "Aurpegi";
      case StringId::Tails:
        return "Gurutze";
      default:
        return nullptr;
    }
  }
}

const char* Pinetime::Applications::Localization::Translate(Language language, StringId id) {
  const auto languageIndex = static_cast<size_t>(language);
  const auto stringIndex = static_cast<size_t>(id);

  if (languageIndex >= translations.size() || stringIndex >= StringCount) {
    return "";
  }

  if (language == Language::Portuguese) {
    if (const auto* translated = TranslatePortuguese(id); translated != nullptr) {
      return translated;
    }
    return english[stringIndex];
  }

  if (language == Language::Russian) {
    if (const auto* translated = TranslateRussian(id); translated != nullptr) {
      return translated;
    }
    return english[stringIndex];
  }

  if (language == Language::French) {
    if (const auto* translated = TranslateFrench(id); translated != nullptr) {
      return translated;
    }
    return english[stringIndex];
  }

  if (language == Language::German) {
    if (const auto* translated = TranslateGerman(id); translated != nullptr) {
      return translated;
    }
    return english[stringIndex];
  }

  if (language == Language::Italian) {
    if (const auto* translated = TranslateItalian(id); translated != nullptr) {
      return translated;
    }
    return english[stringIndex];
  }

  if (language == Language::Turkish) {
    if (const auto* translated = TranslateTurkish(id); translated != nullptr) {
      return translated;
    }
    return english[stringIndex];
  }

  if (language == Language::Polish) {
    if (const auto* translated = TranslatePolish(id); translated != nullptr) {
      return translated;
    }
    return english[stringIndex];
  }

  if (language == Language::Catalan) {
    if (const auto* translated = TranslateCatalan(id); translated != nullptr) {
      return translated;
    }
    return english[stringIndex];
  }

  if (language == Language::Basque) {
    if (const auto* translated = TranslateBasque(id); translated != nullptr) {
      return translated;
    }
    return english[stringIndex];
  }

  return translations[languageIndex][stringIndex];
}
