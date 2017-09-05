/******************************************************************************
 Copyright (C) 2014 by John R. Bradley <jrb@turrettech.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include <memory>
#include <map>

#include "browser-source.hpp"
#include "browser-listener.hpp"
#include "browser-manager.hpp"

#include "browser-source-base.hpp"
#include "browser-source-listener-base.hpp"

#define HOTKEY_RESTART_BUTTON             "hotkey_restart"
#define TEXT_HOTKEY_RESTART_BUTTON        obs_module_text("Vicaption.StopHotkey")

static void hotkey_restart(void *data, obs_hotkey_pair_id id, obs_hotkey_t *hotkey, bool pressed)
{

	if (!pressed) {

		return;
	}

	BrowserSource *bs = static_cast<BrowserSource *>(data);
	BrowserManager::Instance()->RefreshPageNoCache(bs->GetBrowserIdentifier());

	return;
}

BrowserSource::BrowserSource(obs_data_t *settings, obs_source_t *source)
: pimpl(new Impl(this)), source(source), browserIdentifier(0)
{
	this->source = source;
	restart_hotkey_id = obs_hotkey_register_source(
		this->source,
		HOTKEY_RESTART_BUTTON, TEXT_HOTKEY_RESTART_BUTTON,
		hotkey_restart, this);

	UpdateSettings(settings);
}


BrowserSource::~BrowserSource()
{
	if (restart_hotkey_id != NULL) {
		obs_hotkey_unregister(restart_hotkey_id);
	}

	if (browserIdentifier != 0) {
		BrowserManager::Instance()->DestroyBrowser(browserIdentifier);
	}
}

std::shared_ptr<BrowserListener> BrowserSource::CreateListener()
{
	return pimpl->CreateListener();
}

void BrowserSource::RenderActiveTexture(gs_effect_t *effect)
{
	pimpl->RenderCurrentTexture(effect);
}

void BrowserSource::InvalidateActiveTexture()
{
	pimpl->InvalidateActiveTexture();
}

BrowserSource::Impl::Impl(BrowserSource *parent)
: parent(parent), activeTexture(nullptr)
{}

BrowserSource::Impl::~Impl()
{}

void BrowserSource::Impl::RenderCurrentTexture(gs_effect_t *effect)
{
	GetParent()->LockTexture();

	if (activeTexture != nullptr) {
		while (gs_effect_loop(obs_get_base_effect(OBS_EFFECT_PREMULTIPLIED_ALPHA), "Draw"))
			obs_source_draw(activeTexture, 0, 0, 0, 0, false);
	}
	
	GetParent()->UnlockTexture();
}

void BrowserSource::Impl::InvalidateActiveTexture()
{
	activeTexture = nullptr;
}

std::shared_ptr<BrowserListener> BrowserSource::Impl::CreateListener()
{
	return std::shared_ptr<BrowserListener>(
			new BrowserSource::Impl::Listener(this));
}
