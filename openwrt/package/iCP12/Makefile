#
# Copyright (C) 2011-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=iCP12
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/iCP12
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Utility to communicate with a iCP12 Microchip controller
  MAINTAINER:=BayCom GmbH <software@baycom.de>
endef

define Package/iCP12/description
 This package contains a utility which
 can be used to communicate via USB with a iCP12 device.
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(INSTALL_DATA) ./src/iCP12.c ./src/i2c.c $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) -Wall \
		-o $(PKG_BUILD_DIR)/iCP12 $(PKG_BUILD_DIR)/iCP12.c
endef

define Package/iCP12/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/iCP12 $(1)/usr/bin/
endef

$(eval $(call BuildPackage,iCP12))
