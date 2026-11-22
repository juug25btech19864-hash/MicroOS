pub mod dashboard;
pub mod benchmark;
pub mod concurrency;
pub mod snake;
pub mod apt;
pub mod wifi;
pub mod terminal;
pub mod files;

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum AppType {
    Dashboard,
    Benchmark,
    Concurrency,
    Snake,
    Apt,
    Wifi,
    Terminal,
    FileManager,
    FileSystem,
    Home,
}
