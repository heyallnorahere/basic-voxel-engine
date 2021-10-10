import subprocess
import platform
import requests
import tempfile
import os
import os.path as path
VULKAN_VERSION="1.2.182.0"
def set_output(path: str):
    print(f"::set-output name=VULKAN_SDK_VALUE::{path}") # github actions command
def setup_windows():
    INSTALLER_URL=f"https://sdk.lunarg.com/sdk/download/{VULKAN_VERSION}/windows/VulkanSDK-{VULKAN_VERSION}-Installer.exe"
    response = requests.get(INSTALLER_URL, allow_redirects=True)
    tempdir = tempfile.TemporaryDirectory()
    installer_path = path.join(tempdir.name, "vulkan-installer.exe")
    with open(installer_path, "wb") as stream:
        stream.write(response.content)
        stream.close()
    if subprocess.call([ installer_path, "/S" ], shell=True) != 0:
        exit(1)
    set_output(f"C:\\VulkanSDK\\{VULKAN_VERSION}")
def setup_ubuntu():
    PACKAGES = [
        "vulkan-tools",
        "libvulkan-dev",
        "vulkan-validationlayers-dev",
        "spirv-tools"
    ]
    print(f"Installing packages: {str(PACKAGES)}")
    subprocess.call(["sudo", "apt-get", "update"])
    args = [
        "sudo",
        "apt-get",
        "install",
        "-y"
    ]
    for package in PACKAGES:
        args.append(package)
    if subprocess.call(args) != 0:
        exit(1)
    set_output(os.getenv("VULKAN_SDK"))
def setup_macosx():
    DISK_IMAGE_URL=f"https://sdk.lunarg.com/sdk/download/{VULKAN_VERSION}/mac/vulkansdk-macos-{VULKAN_VERSION}.dmg"
    response = requests.get(DISK_IMAGE_URL, allow_redirects=True)
    tempdir = tempfile.TemporaryDirectory()
    disk_image_name = f"vulkansdk-macos-{VULKAN_VERSION}"
    disk_image_path = path.join(tempdir.name, f"{disk_image_name}.dmg")
    with open(disk_image_path, "wb") as stream:
        stream.write(response.content)
        stream.close()
    if subprocess.call(["sudo", "hdiutil", "attach", disk_image_path]) != 0:
        exit(1)
    if subprocess.call(["sudo", f"/Volumes/{disk_image_name}/InstallVulkan.app/Contents/MacOS/InstallVulkan", "in", "--al", "-c"]) != 0:
        exit(1)
    home_dir = os.getenv("HOME")
    set_output(f"{home_dir}/VulkanSDK/{VULKAN_VERSION}/macOS")
def main():
    setup_callbacks = {
        "Windows": setup_windows,
        "Linux": setup_ubuntu,
        "Darwin": setup_macosx
    }
    try:
        system = platform.system()
        callback = setup_callbacks[system]
        print(f"Installing Vulkan for platform: {system}")
        callback()
    except KeyError:
        print("The given platform is not supported! Please see the readme.")
        exit(1)
if __name__ == "__main__":
    main()