import subprocess
import platform
import requests
import tempfile
import os.path as path
def setup_windows():
    INSTALLER_URL="https://sdk.lunarg.com/sdk/download/1.2.182.0/windows/VulkanSDK-1.2.182.0-Installer.exe"
    response = requests.get(INSTALLER_URL, allow_redirects=True)
    tempdir = tempfile.TemporaryDirectory()
    installer_path = path.join(tempdir.name, "vulkan-installer.exe")
    with open(installer_path, "wb") as stream:
        stream.write(response.content)
        stream.close()
    if subprocess.call([ installer_path ], shell=True) != 0:
        exit(1)
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
        "apt-get"
    ]
    for package in PACKAGES:
        args.append(package)
    subprocess.call(args)
def setup_macosx():
    DISK_IMAGE_URL="https://sdk.lunarg.com/sdk/download/1.2.182.0/mac/vulkansdk-macos-1.2.182.0.dmg"
    response = requests.get(DISK_IMAGE_URL, allow_redirects=True)
    tempdir = tempfile.TemporaryDirectory()
    disk_image_name = "vulkan-installer"
    disk_image_path = path.join(tempdir.name, f"{disk_image_name}.dmg")
    with open(disk_image_path, "wb") as stream:
        stream.write(response.content)
        stream.close()
    if subprocess.call(["sudo", "hdiutil", "attach", disk_image_path]) != 0:
        exit(1)
    if subprocess.call(["sudo", f"/Volumes/{disk_image_name}/InstallVulkan.app/Contents/MacOS/InstallVulkan", "in", "--al", "-c"]) != 0:
        exit(1)
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