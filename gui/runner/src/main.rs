use std::path::PathBuf;

fn main() {
    let args: Vec<String> = std::env::args().collect();
    let kernel_path = if args.len() > 1 {
        PathBuf::from(&args[1])
    } else {
        PathBuf::from("target/x86_64-unknown-none/debug/microos-gui-kernel")
    };

    if !kernel_path.exists() {
        eprintln!("[!] Error: Kernel binary not found at {:?}", kernel_path);
        std::process::exit(1);
    }

    let uefi_path = kernel_path.with_extension("uefi.img");
    let bios_path = kernel_path.with_extension("bios.img");

    println!("============================================================");
    println!("     MicroOS GUI Edition UEFI & BIOS Bootable Generator     ");
    println!("============================================================");
    println!("[*] Kernel input: {:?}", kernel_path);

    println!("[*] Generating UEFI GPT disk image: {:?}", uefi_path);
    bootloader::UefiBoot::new(&kernel_path)
        .create_disk_image(&uefi_path)
        .expect("Failed to create UEFI disk image");

    println!("[*] Generating BIOS MBR disk image: {:?}", bios_path);
    bootloader::BiosBoot::new(&kernel_path)
        .create_disk_image(&bios_path)
        .expect("Failed to create BIOS disk image");

    println!("[+] Bootable OS images generated successfully!");
    println!("    UEFI Image : {}", uefi_path.display());
    println!("    BIOS Image : {}", bios_path.display());
}
