use futures::Future;
use serde::{Deserialize, Serialize};
use std::fmt::Display;

pub type StrResult<T = ()> = Result<T, String>;

pub const SESSION_LOG_FNAME: &str = "session_log.txt";
pub const CRASH_LOG_FNAME: &str = "crash_log.txt";

pub fn set_panic_hook() {
    std::panic::set_hook(Box::new(|panic_info| {
        let message = panic_info
            .payload()
            .downcast_ref::<&str>()
            .unwrap_or(&"Unavailable");
        let err_str = format!(
            "Message: {:?}\nBacktrace:\n{:?}",
            message,
            backtrace::Backtrace::new()
        );

        log::error!("{}", err_str);

        #[cfg(not(target_os = "android"))]
        std::thread::spawn(move || {
            msgbox::create("ALVR panicked", &err_str, msgbox::IconType::Error).ok();
        });
    }))
}

// log error and show it in a messagebox (if applicable)
pub fn show_err<T, E: Display>(res: Result<T, E>) -> Result<T, ()> {
    res.map_err(|e| {
        log::error!("{}", e);

        #[cfg(not(target_os = "android"))]
        std::thread::spawn({
            let err_string = e.to_string();
            move || {
                msgbox::create(
                    "ALVR encountered an error",
                    &err_string,
                    msgbox::IconType::Error,
                ).ok();
            }
        });
    })
}

pub fn show_e<E: Display>(e: E) {
    show_err::<(), _>(Err(e)).ok();
}

pub async fn show_err_async<T, E: Display>(
    future_res: impl Future<Output = Result<T, E>>,
) -> Result<T, ()> {
    show_err(future_res.await)
}

#[derive(Serialize, Deserialize, Clone, Copy)]
#[serde(rename_all = "camelCase")]
pub enum SessionUpdateType {
    Settings,
    ClientList,
    Other, // other top level flags, like "setup_wizard"
}

// Log id is serialized as #{ "id": "..." [, "data": ...] }#
// Pound signs are used to identify start and finish of json
#[derive(Serialize, Deserialize)]
#[serde(rename_all = "camelCase", tag = "id", content = "data")]
pub enum LogId {
    #[serde(rename_all = "camelCase")]
    SessionUpdated {
        web_client_id: Option<String>,
        update_type: SessionUpdateType,
    },
    SessionSettingsExtrapolationFailed,
    ClientFoundInvalid,
    ClientFoundWrongVersion(String),
    IncompatibleServer,

    #[serde(rename_all = "camelCase")]
    Statistics {
        total_latency_ms: u32,
        encode_latency_ms: u32,
        decode_latency_ms: u32,
        other_latency_ms: u32,
        client_fps: f32,
        server_fps: f32,
    },

    ClientDisconnected,
}

#[macro_export]
macro_rules! format_id {
    ($id:expr) => {
        format!("#{}#", serde_json::to_string(&$id).unwrap())
    };
}

#[macro_export]
macro_rules! _format_err {
    (@ $($($args:tt)+)?) => {
        format!("At {}:{}", file!(), line!()) $(+ ", " + &format!($($args)+))?
    };
    (id: $id:expr $(, $($args_rest:tt)+)?) => {
        format_id!($id) + " " + &_format_err!(@ $($($args_rest)+)?)
    };
    ($($args:tt)*) => {
        _format_err!(@ $($args)*)
    };
}

#[macro_export]
macro_rules! trace_str {
    ($($args:tt)*) => {
        Err(_format_err!($($args)*))
    };
}

#[macro_export]
macro_rules! trace_err {
    ($res:expr $(, $($args_rest:tt)+)?) => {
        $res.map_err(|e| _format_err!($($($args_rest)+)?) + &format!(": {}", e))
    };
}

// trace_err variant for errors that do not implement fmt::Display
#[macro_export]
macro_rules! trace_err_dbg {
    ($res:expr $(, $($args_rest:tt)+)?) => {
        $res.map_err(|e| _format_err!($($($args_rest)+)?) + &format!(": {:?}", e))
    };
}

#[macro_export]
macro_rules! trace_none {
    ($res:expr $(, $($args_rest:tt)+)?) => {
        $res.ok_or_else(|| _format_err!($($($args_rest)+)?))
    };
}

#[macro_export]
macro_rules! _log {
    (@ $level:expr, $($args:tt)+) => {
        log::log!($level, $($args)+)
    };
    ($level:expr, id: $id:expr $(, $($args_rest:tt)+)?) => {
        _log!(@ $level, "{}", format_id!($id) $(+ " " + &format!($($args_rest)+))?)
    };
    ($level:expr, $($args:tt)+) => {
        _log!(@ $level, $($args)+)
    };
}

#[macro_export]
macro_rules! error {
    ($($args:tt)*) => {
        _log!(log::Level::Error, $($args)*)
    };
}

#[macro_export]
macro_rules! warn {
    ($($args:tt)*) => {
        _log!(log::Level::Warn, $($args)*)
    };
}

#[macro_export]
macro_rules! info {
    ($($args:tt)*) => {
        _log!(log::Level::Info, $($args)*)
    };
}

#[macro_export]
macro_rules! debug {
    ($($args:tt)*) => {
        _log!(log::Level::Debug, $($args)*)
    };
}
