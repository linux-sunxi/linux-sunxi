================================
STEPS TO BUILD KERNEL FOR AAKASH
================================

For Kernel documentation please find ``README_kernel``


COMPILING KERNEL
----------------

1. Clone the repository by ::
    
        git clone https://github.com/androportal/linux-sunxi.git


#. ``cd`` into ``linux-sunxi`` ::

        cd linux-sunxi


#. Checkout to desired branch, it this case ``sunxi-3.0``::

        git checkout sunxi-3.0

#. Compile the a13_configuration ::

        make ARCH=arm a13_defconfig


#. A13-OLinuXino customization can be done using ::

        make ARCH=arm menuconfig


#. Install ``u-boot-tools``::

        sudo apt-get install u-boot-tools


#. To compile kernel download and install ``Sourcery CodeBench Lite Edition``, then issue::

        make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- uImage

#. To make kernel modules ::
        
        make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- INSTALL_MOD_PATH=out modules


#. To install modules in right path ::

        make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- INSTALL_MOD_PATH=out modules_install


#.  After successful compilation ``uImage`` will be available at ``arch/arm/boot/uImage`` and
    also find ``script.bin`` in the root of the directory(This section is subject to change)
    


KERNEL ON SDCARD
----------------

1. Insert ``sdcard`` in card reader or MMC reader available in laptops/netbooks


#. Backup all your sdcard data, and issue the following commands carefully, replace 
   *X* in /dev/sdX with suitable letter(a lower case alphabet) alloted for your
   sdcard, remember *X* will never be *a* if you have a hard disk installed, so keep
   in mind it must be something like ``/dev/sdb`` or /``dev/sdc`` etc. Please check
   twice and if you are still unsure leave this README right now otherwise you may 
   end up with non bootable machine::

        sudo fdisk -u=sectors /dev/sdX


#. Then create partition for kernel and filesystem in your sdcard, from inside ``fdisk`` prompt type ``p`` to list all partitions ::

        Command (m for help): p

#. Now press ``d`` to delete your partition. If you have multiple partitions use partition number
   to delete each partition as shown by fdisk utility::
        
        Command (m for help): d

#. Type ``n`` for new partion(hit enter) and then mention ``p`` for primary partition, hit 
   enter to continue with default starting sector(2048), enter again for last sector and  
   mention ``+16M``, this partition will be later formatted with vfat for kernel::

        Command (m for help): n
        Partition type:
           p   primary (0 primary, 0 extended, 4 free)
           e   extended
        Select (default p): p
        Partition number (1-4, default 1): 
        Using default value 1
        First sector (2048-7744511, default 2048): 
        Using default value 2048
        Last sector, +sectors or +size{K,M,G} (2048-7744511, default 7744511): +16M
 
#. The sdcard is assumed to be 4gb size, similarly create 2nd primary partition using remaining disk space which will serve as filesystem::

        Command (m for help): n
        Partition type:
           p   primary (1 primary, 0 extended, 3 free)
           e   extended
        Select (default p): p
        Partition number (1-4, default 2): 
        Using default value 2
        First sector (34816-7744511, default 34816): 
        Using default value 34816
        Last sector, +sectors or +size{K,M,G} (34816-7744511, default 7744511): 
        Using default value 7744511

#. Now as we have created 2 primary partitions now let's write(w) the changes to disk. For that issue::

        Command (m for help): w

#. As partition is completed, we need to format them respectively, the first partition must be vfat so that uboot can read kernel::

        sudo mkfs.vfat /dev/sdX1

   For rootfs, create ext4 partition::

        sudo mkfs.ext4  /dev/sdX2
        
        
#. Now mount vfat partition to some mount point(directory)::

        sudo mount -t vfat /dev/sdX1 /mnt/boot


   copy kernel to ``/mnt/boot``, assuming that you are still in ``linux-sunxi`` directory::

        sudo cp arch/arm/boot/uImage /mnt/boot

   
   Also copy ``script.bin`` to same directory::

        sudo cp script.bin /mnt/boot

   
   Now, umount /mnt/boot, before that you may want to ``sync`` so that any remaining buffers are written to disks::

        sudo sync

   
   Finally unmount::

        sudo umount /mnt/boot


At this point we have a sdcard readly with kernel. Burn uboot and copy rootfs to make a usuable
Linux for your tablet. 

    
