#!/usr/bin/env python3
"""
MicroOS - Automated Hugging Face Publisher
Uploads MicroOS ISOs, VDIs, documentation, and metadata to Hugging Face.
Student: M. Srivas (USN: 25BTRCL143)
College: Jain University (FET) - AIML
"""

import os
import sys
import argparse
from huggingface_hub import HfApi, login, get_token

def main():
    repo_dir = r"C:\Users\M SRIVAS\.gemini\antigravity\scratch\MicroOS"
    if os.path.exists(repo_dir):
        os.chdir(repo_dir)

    parser = argparse.ArgumentParser(description="Publish MicroOS to Hugging Face")
    parser.add_argument("--repo-id", default=None, help="Hugging Face repo ID (e.g. username/MicroOS)")
    parser.add_argument("--repo-type", default="model", choices=["model", "dataset", "space"], help="Repository type")
    parser.add_argument("--token", default=None, help="Hugging Face write token (or set HF_TOKEN env var)")
    args = parser.parse_args()

    token = args.token or os.environ.get("HF_TOKEN") or get_token()

    print("============================================================")
    print("        MicroOS - Hugging Face Publishing Tool")
    print("============================================================")
    print("Student : M. Srivas (USN: 25BTRCL143)")
    print("College : Jain University (FET) - AIML")
    print("============================================================")

    if not token:
        print("\n[*] To publish to Hugging Face:")
        print("    1. Go to: https://huggingface.co/settings/tokens")
        print("    2. Create a new token with 'Write' role.")
        print()
        try:
            token = input("Enter your Hugging Face Access Token: ").strip()
        except (EOFError, KeyboardInterrupt):
            print("\n[!] Aborted.")
            sys.exit(1)

    if not token:
        print("[!] Token cannot be empty. Exiting.")
        sys.exit(1)

    try:
        login(token=token, add_to_git_credential=True)
        api = HfApi(token=token)
        user_info = api.whoami()
        username = user_info["name"]
        print(f"[+] Successfully authenticated as: {username} ({user_info.get('fullname', '')})")
    except Exception as e:
        print(f"[!] Authentication failed: {e}")
        sys.exit(1)

    target_repo_id = args.repo_id or f"{username}/MicroOS"

    print(f"[*] Target Repository: https://huggingface.co/{target_repo_id} (Type: {args.repo_type})")
    print(f"[*] Creating repository on Hugging Face if not already present...")
    try:
        repo_url = api.create_repo(
            repo_id=target_repo_id,
            repo_type=args.repo_type,
            exist_ok=True,
            private=False
        )
        print(f"[+] Repository ready: {repo_url}")
    except Exception as e:
        print(f"[!] Warning during create_repo: {e}")

    # 1. Upload README.md as the Model Card
    if os.path.exists("README.md"):
        print("[*] Uploading README.md (Repository Card)...")
        api.upload_file(
            path_or_fileobj="README.md",
            path_in_repo="README.md",
            repo_id=target_repo_id,
            repo_type=args.repo_type,
            commit_message="Add MicroOS documentation and repository card - M. Srivas"
        )
        print("[+] Uploaded README.md")

    # 2. Upload Screenshots
    if os.path.exists("documentation/screenshots"):
        print("[*] Uploading documentation screenshots...")
        api.upload_folder(
            folder_path="documentation/screenshots",
            path_in_repo="screenshots",
            repo_id=target_repo_id,
            repo_type=args.repo_type,
            commit_message="Add MicroOS desktop and terminal screenshots"
        )
        print("[+] Uploaded screenshots")

    # 3. Upload Release Binaries (ISOs, VDI, checksums)
    if os.path.exists("release"):
        print("[*] Uploading release ISOs, VDI, and checksums...")
        api.upload_folder(
            folder_path="release",
            path_in_repo="release",
            repo_id=target_repo_id,
            repo_type=args.repo_type,
            commit_message="Upload MicroOS v1.0 release binaries (microos-gui.iso, microos.iso, microos-gui.vdi)"
        )
        print("[+] Uploaded release artifacts")

    print("\n============================================================")
    print("   MICROOS SUCCESSFULLY PUBLISHED TO HUGGING FACE!")
    print("============================================================")
    print(f"  Hugging Face Repository URL : https://huggingface.co/{target_repo_id}")
    print(f"  Direct GUI ISO Download    : https://huggingface.co/{target_repo_id}/resolve/main/release/microos-gui.iso")
    print(f"  Direct CLI ISO Download    : https://huggingface.co/{target_repo_id}/resolve/main/release/microos.iso")
    print(f"  VirtualBox VDI Download    : https://huggingface.co/{target_repo_id}/resolve/main/release/microos-gui.vdi")
    print("============================================================\n")

if __name__ == "__main__":
    main()
