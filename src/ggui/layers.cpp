#include "std_include.hpp"
#include <comdef.h>
#include <commctrl.h>

namespace ggui
{

	// CLayersDlg__vtable
	// CLayer_COMMANDMAP
	// CLayerFunctions

	std::string get_layer_name(const WCHAR* data)
	{
		USES_CONVERSION;
		return W2A(data);
	}

	bool layer_context_open = false;
	bool layer_context_pending = false;
	const clayer_s* context_layer = nullptr;
	const clayer_s* context_layer_last_hovered = nullptr;

	void do_context_menu()
	{
		if(layer_context_open || layer_context_pending)
		{
			ggui::context_menu_style_begin();

			if(!ImGui::IsKeyPressed(ImGuiKey_Escape) && ImGui::BeginPopupContextItem("context_menu##camera"))
			{
				// select layer in og layerwindow
				TreeView_SelectItem(game::layer_dlg->tree.m_hWnd, context_layer);

				layer_context_open = true;
				layer_context_pending = false;

				if (ImGui::MenuItem("Add new layer as child"))
				{
					// CLayers::NewLayer
					utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41C690)(game::layer_dlg);
				}

				if (ImGui::MenuItem("Set as active layer"))
				{
					// CLayers::SetActiveLayer
					utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41C610)(game::layer_dlg);
				}

				ImGui::Separator();

				// same logic as CXYWnd::OnSelectionAddToActiveLayer (grid context menu)
				if (ImGui::MenuItem("Add selection to layer"))
				{
					// CLayersDlg::AsignSelectionToLayer
					utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41C850)(game::layer_dlg);
				}

				if (ImGui::MenuItem("Select everything within layer"))
				{
					// CLayersDlg::SelectAllBrushesInLayer
					utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41C960)(game::layer_dlg);
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Reset layer and children"))
				{
					// CLayersDlg::OnShowLayerAndChildren
					utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41CAF0)(game::layer_dlg);
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Rename layer"))
				{
					// CLayersDlg::RenameLayer
					utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41CC60)(game::layer_dlg);
				}

				if (ImGui::MenuItem("Delete layer"))
				{
					// CLayersDlg::DeleteLayer
					utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41CBE0)(game::layer_dlg);
				}

				ImGui::EndPopup();
			}
			else
			{
				layer_context_open = false;
				layer_context_pending = false;
			}

			ggui::context_menu_style_end();
		}
	}

	void handle_context_menu(const clayer_s* layer)
	{
		context_layer_last_hovered = layer;

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && !layer_context_pending)
		{
			layer_context_pending = true;
			context_layer = layer;
		}
	}

	void do_layer(const clayer_s* layer, bool is_sibling = false)
	{
		if(!layer)
		{
			return;
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding(); // adds height to rows

		int pushed_colors = 0;
		const bool is_active_layer = layer == game::layer_dlg->active_layer;
		const bool is_hidden = layer->enabled == 32;
		const bool is_frozen = layer->enabled == 48;

		if(is_active_layer)
		{
			//ImGui::PushFontFromIndex(E_FONT::BOLD_18PX);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 1.0f, 0.6f, 1.0f)); pushed_colors++;
		}
		else if (is_hidden)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); pushed_colors++;
		}
		else if (is_frozen)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f)); pushed_colors++;
		}

		const std::string layer_string_std = get_layer_name(layer->layer_string);
		const char* layer_string = is_active_layer
					? utils::va("%s   %s", layer_string_std.c_str(), ICON_FA_LAYER_GROUP)
					: utils::va("%s   ", layer_string_std.c_str()); // add a bit of padding to the furthest layer

		// do not show arrow on layers with no children
		const bool is_last_node = !layer->child_layers;

		bool is_hovered = false;
		bool is_pressed = false;

		const auto do_hide_and_freeze = [&]() -> void
		{
			if (is_active_layer)
			{
				ImGui::PopStyleColor(pushed_colors);
				pushed_colors = 0;
			}

			// ----

			ImGui::TableSetColumnIndex(1);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 12.0f);

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 5.0f));
			const ImVec2 btn_size = ImVec2(ImGui::GetFrameHeight() + 6.0f, ImGui::GetFrameHeight());

			if (ImGui::Button(is_hidden ? ICON_FA_EYE_SLASH : ICON_FA_EYE, btn_size))
			{
				if (is_hidden)
				{
					TreeView_SelectItem(game::layer_dlg->tree.m_hWnd, layer);

					// CLayersDlg::OnShowLayer
					utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41CAC0)(game::layer_dlg);
				}
				else
				{
					TreeView_SelectItem(game::layer_dlg->tree.m_hWnd, layer);

					if (is_last_node)
					{
						// CLayersDlg::OnHideLayer
						utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41CA60)(game::layer_dlg);
					}
					else
					{
						// CLayersDlg::OnHideLayerAndChildren
						utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41CA90)(game::layer_dlg);
					}

				}
			}

			ImGui::SameLine(0.0f, 6.0f);

			if (ImGui::Button(ICON_FA_SNOWFLAKE, btn_size))
			{
				if (is_frozen)
				{
					TreeView_SelectItem(game::layer_dlg->tree.m_hWnd, layer);

					// CLayersDlg::OnShowLayer
					utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41CAC0)(game::layer_dlg);
				}
				else
				{
					TreeView_SelectItem(game::layer_dlg->tree.m_hWnd, layer);

					if (is_last_node)
					{
						// CLayersDlg::OnFreezeLayer
						utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41CB20)(game::layer_dlg);
					}
					else
					{
						// CLayersDlg::OnFreezeLayerAndChildren
						utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41CB50)(game::layer_dlg);
					}
				}
			}

			ImGui::PopStyleVar(); // ImGuiStyleVar_FramePadding

			if (pushed_colors)
			{
				ImGui::PopStyleColor(pushed_colors);
				pushed_colors = 0;
			}
		};

		// current layer (custom TreeNodeEx that returns hovered and pressed state - for context menu and bridging to og layerwindow)
		if (ImGui::TreeNodeEx(layer_string, &is_hovered, &is_pressed, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | (is_last_node ? (ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet) : 0)))
		{
			do_hide_and_freeze();

			// ----

			// do all its child layers child siblings
			if(layer->child_layers)
			{
				do_layer(layer->child_layers, false);
			}

			ImGui::TreePop();
		}
		else // if node is collapsed
		{
			do_hide_and_freeze();
		}

		if (pushed_colors)
		{
			ImGui::PopStyleColor(pushed_colors);
			pushed_colors = 0;
		}

		if (is_pressed)
		{
			// select layer in og layerwindow
			TreeView_SelectItem(game::layer_dlg->tree.m_hWnd, layer);
		}

		// IsItemHovered does not work here because do_layers is recursive
		if (is_hovered)
		{
			handle_context_menu(layer);
			//game::printf_to_console("Hovered %s", get_layer_name(layer->layer_string).c_str());

			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				TreeView_SelectItem(game::layer_dlg->tree.m_hWnd, layer);

				// CLayers::SetActiveLayer
				utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41C610)(game::layer_dlg);
			}
		}
		
		// do all siblings for a child
		if(!is_sibling)
		{
			for (auto sibling = layer->sibling_layer_same_level; sibling; sibling = sibling->sibling_layer_same_level)
			{
				do_layer(sibling, true);
			}
		}
	}

	bool layer_dialog::gui()
	{
		ImGui::SetNextWindowSize(ImVec2(400.0f, 600.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Layers##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar))
		{
			SPACING(0.0f, 2.0f);

			if (ImGui::BeginTable("split", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_ScrollX))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::AlignTextToFramePadding();

				ImGui::PushFontFromIndex(BOLD_18PX);
				ImGui::Text("    The Map");
				ImGui::PopFont();

				//if (ImGui::TreeNodeEx(get_layer_name(game::layer_dlg->root_node_the_map->layer_string).c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth))
				{
					// do all siblings of the first child (same level as the first use-able layer)
					for (auto child = game::layer_dlg->root_node_the_map->child_layers; child; child = child->sibling_layer_same_level)
					{
						do_layer(child, true);
					}

					//ImGui::TreePop();
				}
				
				ImGui::EndTable();
			}

			do_context_menu();

			ImGui::End();
			return true;
		}

		return false;
	}

	// CMainFrame::OnLayersDlg
	void layer_dialog::on_layerdialog_command()
	{
		const auto gui = GET_GUI(ggui::layer_dialog);

		if (gui->is_inactive_tab() && gui->is_active())
		{
			gui->set_bring_to_front(true);
			return;
		}

		gui->toggle();
	}

	void layer_dialog::on_open()
	{ }

	void layer_dialog::on_close()
	{ }

	void layer_dialog::hooks()
	{
		// detour CMainFrame::OnLayersDlg (hotkey to open the original dialog)
		utils::hook::detour(0x42BD10, layer_dialog::on_layerdialog_command, HK_JUMP);
	}

	REGISTER_GUI(layer_dialog);
}
