// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ImGui GLFW binding with OpenGL
// You can copy and use unmodified imgui_impl_* files in your project. See
// main.cpp for an example of using this. If you use this binding you'll need to
// call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(),
// ImGui::Render() and ImGui_ImplXXXX_Shutdown(). If you are new to ImGui, see
// examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

#include "ImGuiOrbit.h"

#include <imgui.h>

#include "Capture.h"
#include "Core.h"
#include "GlCanvas.h"
#include "Images.h"
#include "Message.h"
#include "OpenGl.h"
#include "Params.h"
#include "Pdb.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Data
struct GLFWwindow {};
// static GLFWwindow*  g_Window = NULL;
static bool g_MousePressed[3] = {false, false, false};
static float g_MouseWheel = 0.0f;
static GLuint g_FontTexture = 0;

GLuint GTextureInjected = 0;
GLuint GTextureTimer = 0;
GLuint GTextureHelp = 0;
GLuint GTextureRecord = 0;

// This is the main rendering function that you have to implement and provide to
// ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure) If text
// or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by
// (0.5f,0.5f) or (0.375f,0.375f)
void Orbit_ImGui_RenderDrawLists(ImDrawData* draw_data) {
  // We are using the OpenGL fixed pipeline to make the example code simpler to
  // read! Setup render state: alpha-blending enabled, no face culling, no depth
  // testing, scissor enabled, vertex/texcoord/color pointers.
  GLint last_texture;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  GLint last_viewport[4];
  glGetIntegerv(GL_VIEWPORT, last_viewport);
  glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  // glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glEnable(GL_TEXTURE_2D);
  glUseProgram(
      0);  // You may want this if using this code in an OpenGL 3+ context

  // Handle cases of screen coordinates != from framebuffer coordinates (e.g.
  // retina displays)
  ImGuiIO& io = ImGui::GetIO();
  int fb_width =
      static_cast<int>(io.DisplaySize.x * io.DisplayFramebufferScale.x);
  int fb_height =
      static_cast<int>(io.DisplaySize.y * io.DisplayFramebufferScale.y);
  draw_data->ScaleClipRects(io.DisplayFramebufferScale);

  // Setup viewport, orthographic projection matrix
  glViewport(0, 0, static_cast<GLsizei>(fb_width),
             static_cast<GLsizei>(fb_height));
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, -1.0f, +1.0f);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // Render command lists
  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const ImDrawList* cmd_list = draw_data->CmdLists[n];
    const uint8_t* vtx_buffer =
        reinterpret_cast<const uint8_t*>(&cmd_list->VtxBuffer.front());
    const ImDrawIdx* idx_buffer = &cmd_list->IdxBuffer.front();
    glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert),
                    vtx_buffer + offsetof(ImDrawVert, pos));
    glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert),
                      vtx_buffer + offsetof(ImDrawVert, uv));
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert),
                   vtx_buffer + offsetof(ImDrawVert, col));

    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++) {
      const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
      if (pcmd->UserCallback) {
        pcmd->UserCallback(cmd_list, pcmd);
      } else {
        glBindTexture(
            GL_TEXTURE_2D,
            static_cast<GLuint>(reinterpret_cast<intptr_t>(pcmd->TextureId)));
        glScissor(static_cast<int>(pcmd->ClipRect.x),
                  static_cast<int>(fb_height - pcmd->ClipRect.w),
                  static_cast<int>(pcmd->ClipRect.z - pcmd->ClipRect.x),
                  static_cast<int>(pcmd->ClipRect.w - pcmd->ClipRect.y));
        glDrawElements(
            GL_TRIANGLES, static_cast<GLsizei>(pcmd->ElemCount),
            sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
            idx_buffer);
      }
      idx_buffer += pcmd->ElemCount;
    }
  }

  // Restore modified state
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glBindTexture(GL_TEXTURE_2D, last_texture);
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
  glViewport(last_viewport[0], last_viewport[1],
             static_cast<GLsizei>(last_viewport[2]),
             static_cast<GLsizei>(last_viewport[3]));
}

const char* Orbit_ImGui_GetClipboardText() {
  return nullptr;  // return glfwGetClipboardString(g_Window);
}

void Orbit_ImGui_SetClipboardText(const char*) {
  // TODO
}

void Orbit_ImGui_MouseButtonCallback(GlCanvas* a_GlCanvas, int button,
                                     bool down) {
  ScopeImguiContext state(a_GlCanvas->GetImGuiContext());
  if (button >= 0 && button < 3) {
    g_MousePressed[button] = down;
  }
}

void Orbit_ImGui_ScrollCallback(GlCanvas* a_GlCanvas, int scroll) {
  ScopeImguiContext state(a_GlCanvas->GetImGuiContext());
  g_MouseWheel += scroll;  // Use fractional mouse wheel, 1.0 unit 5 lines.
}

void Orbit_ImGui_KeyCallback(GlCanvas* a_Canvas, int key, bool down) {
  // Convert "enter" into "return"
  if (key == 5) key = 4;

  ScopeImguiContext state(a_Canvas->GetImGuiContext());
  if (key >= 0 && key < 512) {
    ImGuiIO& io = ImGui::GetIO();
    io.KeysDown[key] = down;
  }
}

void Orbit_ImGui_CharCallback(GlCanvas* a_GlCanvas, unsigned int c) {
  ScopeImguiContext state(a_GlCanvas->GetImGuiContext());
  ImGuiIO& io = ImGui::GetIO();
  if (c > 0 && c < 0x10000) io.AddInputCharacter(static_cast<uint16_t>(c));
}

// Simple helper function to load an image into a OpenGL texture with common
// settings
bool LoadTextureFromFile(const char* filename, uint32_t* out_texture,
                         int* out_width, int* out_height) {
  // Load from file
  int image_width = 0;
  int image_height = 0;
  unsigned char* image_data =
      stbi_load(filename, &image_width, &image_height, NULL, 4);
  if (image_data == NULL) return false;

  // Create a OpenGL texture identifier
  GLuint image_texture;
  glGenTextures(1, &image_texture);
  glBindTexture(GL_TEXTURE_2D, image_texture);

  // Setup filtering parameters for display
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Upload pixels into texture
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, image_data);
  stbi_image_free(image_data);

  *out_texture = image_texture;
  *out_width = image_width;
  *out_height = image_height;

  return true;
}

uint32_t LoadTextureFromFile(const char* file_name) {
  uint32_t texture_id = 0;
  int image_width = 0;
  int image_height = 0;
  if (!LoadTextureFromFile(file_name, &texture_id, &image_width,
                           &image_height)) {
    LOG("ERROR, could not load texture %s", file_name);
  }

  return texture_id;
}

bool Orbit_ImGui_CreateDeviceObjects() {
  // Build texture atlas
  ImGuiIO& io = ImGui::GetIO();
  unsigned char* pixels;
  int width, height;
  io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

  // Upload texture to graphics system
  GLint last_texture;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  glGenTextures(1, &g_FontTexture);
  glBindTexture(GL_TEXTURE_2D, g_FontTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA,
               GL_UNSIGNED_BYTE, pixels);

  glGenTextures(1, &GTextureInjected);
  glBindTexture(GL_TEXTURE_2D, GTextureInjected);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, inject_image.width,
               inject_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               inject_image.pixel_data);

  glGenTextures(1, &GTextureTimer);
  glBindTexture(GL_TEXTURE_2D, GTextureTimer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, inject_image.width,
               inject_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               timer_image.pixel_data);

  glGenTextures(1, &GTextureHelp);
  glBindTexture(GL_TEXTURE_2D, GTextureHelp);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, help_image.width, help_image.height,
               0, GL_RGBA, GL_UNSIGNED_BYTE, help_image.pixel_data);

  glGenTextures(1, &GTextureRecord);
  glBindTexture(GL_TEXTURE_2D, GTextureRecord);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, record_image.width,
               record_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               record_image.pixel_data);

  // Restore state
  glBindTexture(GL_TEXTURE_2D, last_texture);

  return true;
}

void Orbit_ImGui_InvalidateDeviceObjects() {
  if (g_FontTexture) {
    glDeleteTextures(1, &g_FontTexture);
    ImGui::GetIO().Fonts->TexID = 0;
    g_FontTexture = 0;
  }
}

ImFont* GOrbitImguiFont;

//-----------------------------------------------------------------------------
// ProggyClean.ttf
// Copyright (c) 2004, 2005 Tristan Grimmer
// MIT license (see License.txt in
// http://www.upperbounds.net/download/ProggyClean.ttf.zip) Download and more
// information at http://upperbounds.net
//-----------------------------------------------------------------------------
// File: 'ProggyClean.ttf' (41208 bytes)
// Exported using binary_to_compressed_c.cpp
//-----------------------------------------------------------------------------
static const char proggy_clean_ttf_compressed_data_base85[11980 + 1] =
    "7])#######hV0qs'/###[),##/l:$#Q6>##5[n42>c-TH`->>#/"
    "e>11NNV=Bv(*:.F?uu#(gRU.o0XGH`$vhLG1hxt9?W`#,5LsCp#-i>.r$<$6pD>Lb';"
    "9Crc6tgXmKVeU2cD4Eo3R/"
    "2*>]b(MC;$jPfY.;h^`IWM9<Lh2TlS+f-s$o6Q<BWH`YiU.xfLq$N;$0iR/GX:U(jcW2p/"
    "W*q?-qmnUCI;jHSAiFWM.R*kU@C=GH?a9wp8f$e.-4^Qg1)Q-GL(lf(r/7GrRgwV%MS=C#"
    "`8ND>Qo#t'X#(v#Y9w0#1D$CIf;W'#pWUPXOuxXuU(H9M(1<q-UE31#^-V'8IRUo7Qf./"
    "L>=Ke$$'5F%)]0^#0X@U.a<r:QLtFsLcL6##lOj)#.Y5<-R&KgLwqJfLgN&;Q?gI^#DY2uL"
    "i@^rMl9t=cWq6##weg>$FBjVQTSDgEKnIS7EM9>ZY9w0#L;>>#Mx&4Mvt//"
    "L[MkA#W@lK.N'[0#7RL_&#w+F%HtG9M#XL`N&.,GM4Pg;-<nLENhvx>-VsM.M0rJfLH2eTM`*"
    "oJMHRC`N"
    "kfimM2J,W-jXS:)r0wK#@Fge$U>`w'N7G#$#fB#$E^$#:9:hk+eOe--6x)F7*E%?76%^"
    "GMHePW-Z5l'&GiF#$956:rS?dA#fiK:)Yr+`&#0j@'DbG&#^$PG.Ll+DNa<XCMKEV*N)LN/N"
    "*b=%Q6pia-Xg8I$<MR&,VdJe$<(7G;Ckl'&hF;;$<_=X(b.RS%%)###MPBuuE1V:v&cX&#2m#("
    "&cV]`k9OhLMbn%s$G2,B$BfD3X*sp5#l,$R#]x_X1xKX%b5U*[r5iMfUo9U`N99hG)"
    "tm+/Us9pG)XPu`<0s-)WTt(gCRxIg(%6sfh=ktMKn3j)<6<b5Sk_/0(^]AaN#(p/"
    "L>&VZ>1i%h1S9u5o@YaaW$e+b<TWFn/"
    "Z:Oh(Cx2$lNEoN^e)#CFY@@I;BOQ*sRwZtZxRcU7uW6CX"
    "ow0i(?$Q[cjOd[P4d)]>ROPOpxTO7Stwi1::iB1q)C_=dV26J;2,]7op$]uQr@_V7$q^%"
    "lQwtuHY]=DX,n3L#0PHDO4f9>dC@O>HBuKPpP*E,N+b3L#lpR/MrTEH.IAQk.a>D[.e;mc."
    "x]Ip.PH^'/aqUO/$1WxLoW0[iLA<QT;5HKD+@qQ'NQ(3_PLhE48R.qAPSwQ0/"
    "WK?Z,[x?-J;jQTWA0X@KJ(_Y8N-:/M74:/"
    "-ZpKrUss?d#dZq]DAbkU*JqkL+nwX@@47`5>w=4h(9.`G"
    "CRUxHPeR`5Mjol(dUWxZa(>STrPkrJiWx`5U7F#.g*jrohGg`cg:lSTvEY/"
    "EV_7H4Q9[Z%cnv;JQYZ5q.l7Zeas:HOIZOB?G<Nald$qs]@]L<J7bR*>gv:[7MI2k).'2($"
    "5FNP&EQ(,)"
    "U]W]+fh18.vsai00);D3@4ku5P?DP8aJt+;qUM]=+b'8@;mViBKx0DE[-auGl8:PJ&Dj+M6OC]"
    "O^((##]`0i)drT;-7X`=-H3[igUnPG-NZlo.#k@h#=Ork$m>a>$-?Tm$UV(?#P6YY#"
    "'/###xe7q.73rI3*pP/$1>s9)W,JrM7SN]'/"
    "4C#v$U`0#V.[0>xQsH$fEmPMgY2u7Kh(G%siIfLSoS+MK2eTM$=5,M8p`A.;_R%#u[K#$"
    "x4AG8.kK/HSB==-'Ie/QTtG?-.*^N-4B/ZM"
    "_3YlQC7(p7q)&](`6_c)$/"
    "*JL(L-^(]$wIM`dPtOdGA,U3:w2M-0<q-]L_?^)1vw'.,MRsqVr.L;aN&#/"
    "EgJ)PBc[-f>+WomX2u7lqM2iEumMTcsF?-aT=Z-97UEnXglEn1K-bnEO`gu"
    "Ft(c%=;Am_Qs@jLooI&NX;]0#j4#F14;gl8-GQpgwhrq8'=l_f-b49'UOqkLu7-##oDY2L(te+"
    "Mch&gLYtJ,MEtJfLh'x'M=$CS-ZZ%P]8bZ>#S?YY#%Q&q'3^Fw&?D)UDNrocM3A76/"
    "/oL?#h7gl85[qW/"
    "NDOk%16ij;+:1a'iNIdb-ou8.P*w,v5#EI$TWS>Pot-R*H'-SEpA:g)f+O$%%`kA#G=8RMmG1&"
    "O`>to8bC]T&$,n.LoO>29sp3dt-52U%VM#q7'DHpg+#Z9%H[K<L"
    "%a2E-grWVM3@2=-k22tL]4$##6We'8UJCKE[d_=%wI;'6X-GsLX4j^SgJ$##R*w,vP3wK#iiW&"
    "#*h^D&R?jp7+/u&#(AP##XU8c$fSYW-J95_-Dp[g9wcO&#M-h1OcJlc-*vpw0xUX&#"
    "OQFKNX@QI'IoPp7nb,QU//"
    "MQ&ZDkKP)X<WSVL(68uVl&#c'[0#(s1X&xm$Y%B7*K:eDA323j998GXbA#pwMs-jgD$9QISB-"
    "A_(aN4xoFM^@C58D0+Q+q3n0#3U1InDjF682-SjMXJK)("
    "h$hxua_K]ul92%'BOU&#BRRh-slg8KDlr:%L71Ka:.A;%YULjDPmL<LYs8i#XwJOYaKPKc1h:'"
    "9Ke,g)b),78=I39B;xiY$bgGw-&.Zi9InXDuYa%G*f2Bq7mn9^#p1vv%#(Wi-;/Z5h"
    "o;#2:;%d&#x9v68C5g?ntX0X)pT`;%pB3q7mgGN)3%(P8nTd5L7GeA-GL@+%J3u2:(Yf>et`e;"
    ")f#Km8&+DC$I46>#Kr]]u-[=99tts1.qb#q72g1WJO81q+eN'03'eM>&1XxY-caEnO"
    "j%2n8)),?ILR5^.Ibn<-X-Mq7[a82Lq:F&#ce+S9wsCK*x`569E8ew'He]h:sI[2LM$["
    "guka3ZRd6:t%IG:;$%YiJ:Nq=?eAw;/:nnDq0(CYcMpG)qLN4$##&J<j$UpK<Q4a1]MupW^-"
    "sj_$%[HK%'F####QRZJ::Y3EGl4'@%FkiAOg#p[##O`gukTfBHagL<LHw%q&OV0##F=6/"
    ":chIm0@eCP8X]:kFI%hl8hgO@RcBhS-@Qb$%+m=hPDLg*%K8ln(wcf3/'DW-$.lR?n[nCH-"
    "eXOONTJlh:.RYF%3'p6sq:UIMA945&^HFS87@$EP2iG<-lCO$%c`uKGD3rC$x0BL8aFn--`ke%"
    "#HMP'vh1/R&O_J9'um,.<tx[@%wsJk&bUT2`0uMv7gg#qp/ij.L56'hl;.s5CUrxjO"
    "M7-##.l+Au'A&O:-T72L]P`&=;ctp'XScX*rU.>-XTt,%OVU4)S1+R-#dg0/"
    "Nn?Ku1^0f$B*P:Rowwm-`0PKjYDDM'3]d39VZHEl4,.j']Pk-M.h^&:0FACm$maq-&sgw0t7/"
    "6(^xtk%"
    "LuH88Fj-ekm>GA#_>568x6(OFRl-IZp`&b,_P'$M<Jnq79VsJW/mWS*PUiq76;]/"
    "NM_>hLbxfc$mj`,O;&%W2m`Zh:/"
    ")Uetw:aJ%]K9h:TcF]u_-Sj9,VK3M.*'&0D[Ca]J9gp8,kAW]"
    "%(?A%R$f<->Zts'^kn=-^@c4%-pY6qI%J%1IGxfLU9CP8cbPlXv);C=b),<2mOvP8up,"
    "UVf3839acAWAW-W?#ao/^#%KYo8fRULNd2.>%m]UK:n%r$'sw]J;5pAoO_#2mO3n,'=H5(et"
    "Hg*`+RLgv>=4U8guD$I%D:W>-r5V*%j*W:Kvej.Lp$<M-SGZ':+Q_k+uvOSLiEo(<aD/"
    "K<CCc`'Lx>'?;++O'>()jLR-^u68PHm8ZFWe+ej8h:9r6L*0//c&iH&R8pRbA#Kjm%upV1g:"
    "a_#Ur7FuA#(tRh#.Y5K+@?3<-8m0$PEn;J:rh6?I6uG<-`wMU'ircp0LaE_OtlMb&1#6T.#"
    "FDKu#1Lw%u%+GM+X'e?YLfjM[VO0MbuFp7;>Q&#WIo)0@F%q7c#4XAXN-U&VB<HFF*qL("
    "$/V,;(kXZejWO`<[5?\?ewY(*9=%wDc;,u<'9t3W-(H1th3+G]ucQ]kLs7df($/"
    "*JL]@*t7Bu_G3_7mp7<iaQjO@.kLg;x3B0lqp7Hf,^Ze7-##@/"
    "c58Mo(3;knp0%)A7?-W+eI'o8)b<"
    "nKnw'Ho8C=Y>pqB>0ie&jhZ[?iLR@@_AvA-iQC(=ksRZRVp7`.=+NpBC%rh&3]R:8XDmE5^"
    "V8O(x<<aG/1N$#FX$0V5Y6x'aErI3I$7x%E`v<-BY,)%-?Psf*l?%C3.mM(=/M0:JxG'?"
    "7WhH%o'a<-80g0NBxoO(GH<dM]n.+%q@jH?f.UsJ2Ggs&4<-e47&Kl+f//"
    "9@`b+?.TeN_&B8Ss?v;^Trk;f#YvJkl&w$]>-+k?'(<S:68tq*WoDfZu';mM?8X[ma8W%*`-=;"
    "D.(nc7/;"
    ")g:T1=^J$&BRV(-lTmNB6xqB[@0*o.erM*<SWF]u2=st-*(6v>^](H.aREZSi,#1:[IXaZFOm<"
    "-ui#qUq2$##Ri;u75OK#(RtaW-K-F`S+cF]uN`-KMQ%rP/Xri.LRcB##=YL3BgM/3M"
    "D?@f&1'BW-)Ju<L25gl8uhVm1hL$##*8###'A3/"
    "LkKW+(^rWX?5W_8g)a(m&K8P>#bmmWCMkk&#TR`C,5d>g)F;t,4:@_l8G/"
    "5h4vUd%&%950:VXD'QdWoY-F$BtUwmfe$YqL'8(PWX("
    "P?^@Po3$##`MSs?DWBZ/S>+4%>fX,VWv/w'KD`LP5IbH;rTV>n3cEK8U#bX]l-/"
    "V+^lj3;vlMb&[5YQ8#pekX9JP3XUC72L,,?+Ni&co7ApnO*5NK,((W-i:$,kp'UDAO("
    "G0Sq7MVjJs"
    "bIu)'Z,*[>br5fX^:FPAWr-m2KgL<LUN098kTF&#lvo58=/vjDo;.;)Ka*hLR#/"
    "k=rKbxuV`>Q_nN6'8uTG&#1T5g)uLv:873UpTLgH+#FgpH'_o1780Ph8KmxQJ8#H72L4@768@"
    "Tm&Q"
    "h4CB/5OvmA&,Q&QbUoi$a_%3M01H)4x7I^&KQVgtFnV+;[Pc>[m4k//"
    ",]1?#`VY[Jr*3&&slRfLiVZJ:]?=K3Sw=[$=uRB?3xk48@aeg<Z'<$#4H)6,>e0jT6'N#(q%."
    "O=?2S]u*(m<-"
    "V8J'(1)G][68hW$5'q[GC&5j`TE?m'esFGNRM)j,ffZ?-qx8;->g4t*:CIP/[Qap7/"
    "9'#(1sao7w-.qNUdkJ)tCF&#B^;xGvn2r9FEPFFFcL@.iFNkTve$m%#QvQS8U@)2Z+3K:AKM5i"
    "sZ88+dKQ)W6>J%CL<KE>`.d*(B`-n8D9oK<Up]c$X$(,)M8Zt7/"
    "[rdkqTgl-0cuGMv'?>-XV1q['-5k'cAZ69e;D_?$ZPP&s^+7])$*$#@QYi9,5P&#9r+$%CE="
    "68>K8r0=dSC%%(@p7"
    ".m7jilQ02'0-VWAg<a/''3u.=4L$Y)6k/K:_[3=&jvL<L0C/"
    "2'v:^;-DIBW,B4E68:kZ;%?8(Q8BH=kO65BW?xSG&#@uU,DS*,?.+(o(#1vCS8#CHF>TlGW'b)"
    "Tq7VT9q^*^$$.:&N@@"
    "$&)WHtPm*5_rO0&e%K&#-30j(E4#'Zb.o/"
    "(Tpm$>K'f@[PvFl,hfINTNU6u'0pao7%XUp9]5.>%h`8_=VYbxuel.NTSsJfLacFu3B'lQSu/"
    "m6-Oqem8T+oE--$0a/k]uj9EwsG>%veR*"
    "hv^BFpQj:K'#SJ,sB-'#](j.Lg92rTw-*n%@/;39rrJF,l#qV%OrtBeC6/"
    ",;qB3ebNW[?,Hqj2L.1NP&GjUR=1D8QaS3Up&@*9wP?+lo7b?@%'k4`p0Z$22%K3+iCZj?"
    "XJN4Nm&+YF]u"
    "@-W$U%VEQ/,,>>#)D<h#`)h0:<Q6909ua+&VU%n2:cG3FJ-%@Bj-DgLr`Hw&HAKjKjseK</"
    "xKT*)B,N9X3]krc12t'pgTV(Lv-tL[xg_%=M_q7a^x?7Ubd>#%8cY#YZ?=,`Wdxu/ae&#"
    "w6)R89tI#6@s'(6Bf7a&?S=^ZI_kS&ai`&=tE72L_D,;^R)7[$s<Eh#c&)q.MXI%#"
    "v9ROa5FZO%sF7q7Nwb&#ptUJ:aqJe$Sl68%.D###EC><?-aF&#RNQv>o8lKN%5/"
    "$(vdfq7+ebA#"
    "u1p]ovUKW&Y%q]'>$1@-[xfn$7ZTp7mM,G,Ko7a&Gu%G[RMxJs[0MM%wci.LFDK)(<c`Q8N)"
    "jEIF*+?P2a8g%)$q]o2aH8C&<SibC/q,(e:v;-b#6[$NtDZ84Je2KNvB#$P5?tQ3nt(0"
    "d=j.LQf./"
    "Ll33+(;q3L-w=8dX$#WF&uIJ@-bfI>%:_i2B5CsR8&9Z&#=mPEnm0f`<&c)QL5uJ#%u%lJj+D-"
    "r;BoF&#4DoS97h5g)E#o:&S4weDF,9^Hoe`h*L+_a*NrLW-1pG_&2UdB8"
    "6e%B/:=>)N4xeW.*wft-;$'58-ESqr<b?UI(_%@[P46>#U`'6AQ]m&6/"
    "`Z>#S?YY#Vc;r7U2&326d=w&H####?TZ`*4?&.MK?LP8Vxg>$[QXc%QJv92.(Db*B)gb*"
    "BM9dM*hJMAo*c&#"
    "b0v=Pjer]$gG&JXDf->'StvU7505l9$AFvgYRI^&<^b68?j#q9QX4SM'RO#&sL1IM."
    "rJfLUAj221]d##DW=m83u5;'bYx,*Sl0hL(W;;$doB&O/TQ:(Z^xBdLjL<Lni;''X.`$#8+1GD"
    ":k$YUWsbn8ogh6rxZ2Z9]%nd+>V#*8U_72Lh+2Q8Cj0i:6hp&$C/"
    ":p(HK>T8Y[gHQ4`4)'$Ab(Nof%V'8hL&#<NEdtg(n'=S1A(Q1/"
    "I&4([%dM`,Iu'1:_hL>SfD07&6D<fp8dHM7/g+"
    "tlPN9J*rKaPct&?'uBCem^jn%9_K)<,C5K3s=5g&GmJb*[SYq7K;TRLGCsM-$$;S%:Y@"
    "r7AK0pprpL<Lrh,q7e/%KWK:50I^+m'vi`3?%Zp+<-d+$L-Sv:@.o19n$s0&39;kn;S%BSq*"
    "$3WoJSCLweV[aZ'MQIjO<7;X-X;&+dMLvu#^UsGEC9WEc[X(wI7#2.(F0jV*eZf<-Qv3J-c+"
    "J5AlrB#$p(H68LvEA'q3n0#m,[`*8Ft)FcYgEud]CWfm68,(aLA$@EFTgLXoBq/UPlp7"
    ":d[/"
    ";r_ix=:TF`S5H-b<LI&HY(K=h#)]Lk$K14lVfm:x$H<3^Ql<M`$OhapBnkup'D#L$Pb_`N*g]"
    "2e;X/Dtg,bsj&K#2[-:iYr'_wgH)NUIR8a1n#S?Yej'h8^58UbZd+^FKD*T@;6A"
    "7aQC[K8d-(v6GI$x:T<&'Gp5Uf>@M.*J:;$-rv29'M]8qMv-tLp,'886iaC=Hb*YJoKJ,(j%K="
    "H`K.v9HggqBIiZu'QvBT.#=)0ukruV&.)3=(^1`o*Pj4<-<aN((^7('#Z0wK#5GX@7"
    "u][`*S^43933A4rl][`*O4CgLEl]v$1Q3AeF37dbXk,.)vj#x'd`;qgbQR%FW,2(?LO=s%"
    "Sc68%NP'##Aotl8x=BE#j1UD([3$M(]UI2LX3RpKN@;/#f'f/&_mt&F)XdF<9t4)Qa.*kT"
    "LwQ'(TTB9.xH'>#MJ+gLq9-##@HuZPN0]u:h7.T..G:;$/"
    "Usj(T7`Q8tT72LnYl<-qx8;-HV7Q-&Xdx%1a,hC=0u+HlsV>nuIQL-5<N?)NBS)QN*_I,?&)2'"
    "IM%L3I)X((e/dl2&8'<M"
    ":^#M*Q+[T.Xri.LYS3v%fF`68h;b-X[/En'CR.q7E)p'/"
    "kle2HM,u;^%OKC-N+Ll%F9CF<Nf'^#t2L,;27W:0O@6##U6W7:$rJfLWHj$#)woqBefIZ.PK<"
    "b*t7ed;p*_m;4ExK#h@&]>"
    "_>@kXQtMacfD.m-VAb8;IReM3$wf0''hra*so568'Ip&vRs849'MRYSp%:t:h5qSgwpEr$B>Q,"
    ";s(C#$)`svQuF$##-D,##,g68@2[T;.XSdN9Qe)rpt._K-#5wF)sP'##p#C0c%-Gb%"
    "hd+<-j'Ai*x&&HMkT]C'OSl##5RG[JXaHN;d'uA#x._U;.`PU@(Z3dt4r152@:v,'R.Sj'w#0<"
    "-;kPI)FfJ&#AYJ&#//)>-k=m=*XnK$>=)72L]0I%>.G690a:$##<,);?;72#?x9+d;"
    "^V'9;jY@;)br#q^YQpx:X#Te$Z^'=-=bGhLf:D6&bNwZ9-ZD#n^9HhLMr5G;']d&6'wYmTFmL<"
    "LD)F^%[tC'8;+9E#C$g%#5Y>q9wI>P(9mI[>kC-ekLC/R&CH+s'B;K-M6$EB%is00:"
    "+A4[7xks.LrNk0&E)wILYF@2L'0Nb$+pv<(2.768/"
    "FrY&h$^3i&@+G%JT'<-,v`3;_)I9M^AE]CN?Cl2AZg+%4iTpT3<n-&%H%b<FDj2M<hH=&Eh<"
    "2Len$b*aTX=-8QxN)k11IM1c^j%"
    "9s<L<NFSo)B?+<-(GxsF,^-Eh@$4dXhN$+#rxK8'je'D7k`e;)2pYwPA'_p9&@^18ml1^[@"
    "g4t*[JOa*[=Qp7(qJ_oOL^('7fB&Hq-:sf,sNj8xq^>$U4O]GKx'm9)b@p7YsvK3w^YR-"
    "CdQ*:Ir<($u&)#(&?L9Rg3H)4fiEp^iI9O8KnTj,]H?D*r7'M;PwZ9K0E^k&-cpI;.p/"
    "6_vwoFMV<->#%Xi.LxVnrU(4&8/P+:hLSKj$#U%]49t'I:rgMi'FL@a:0Y-uA[39',(vbma*"
    "hU%<-SRF`Tt:542R_VV$p@[p8DV[A,?1839FWdF<TddF<9Ah-6&9tWoDlh]&1SpGMq>Ti1O*H&"
    "#(AL8[_P%.M>v^-))qOT*F5Cq0`Ye%+$B6i:7@0IX<N+T+0MlMBPQ*Vj>SsD<U4JHY"
    "8kD2)2fU/M#$e.)T4,_=8hLim[&);?UkK'-x?'(:siIfL<$pFM`i<?%W(mGDHM%>iWP,##P`%/"
    "L<eXi:@Z9C.7o=@(pXdAO/NLQ8lPl+HPOQa8wD8=^GlPa8TKI1CjhsCTSLJM'/Wl>-"
    "S(qw%sf/@%#B6;/"
    "U7K]uZbi^Oc^2n<bhPmUkMw>%t<)'mEVE''n`WnJra$^TKvX5B>;_aSEK',(hwa0:i4G?.Bci."
    "(X[?b*($,=-n<.Q%`(X=?+@Am*Js0&=3bh8K]mL<LoNs'6,'85`"
    "0?t/'_U59@]ddF<#LdF<eWdF<OuN/45rY<-L@&#+fm>69=Lb,OcZV/"
    ");TTm8VI;?%OtJ<(b4mq7M6:u?KRdF<gR@2L=FNU-<b[(9c/"
    "ML3m;Z[$oF3g)GAWqpARc=<ROu7cL5l;-[A]%/"
    "+fsd;l#SafT/"
    "f*W]0=O'$(Tb<[)*@e775R-:Yob%g*>l*:xP?Yb.5)%w_I?7uk5JC+FS(m#i'k.'a0i)9<7b'"
    "fs'59hq$*5Uhv##pi^8+hIEBF`nvo`;'l0.^S1<-wUK2/Coh58KKhLj"
    "M=SO*rfO`+qC`W-On.=AJ56>>i2@2LH6A:&5q`?9I3@@'04&p2/"
    "LVa*T-4<-i3;M9UvZd+N7>b*eIwg:CC)c<>nO&#<IGe;__.thjZl<%w(Wk2xmp4Q@I#I9,DF]"
    "u7-P=.-_:YJ]aS@V"
    "?6*C()dOp7:WL,b&3Rg/"
    ".cmM9&r^>$(>.Z-I&J(Q0Hd5Q%7Co-b`-c<N(6r@ip+AurK<m86QIth*#v;-OBqi+L7wDE-"
    "Ir8K['m+DDSLwK&/.?-V%U_%3:qKNu$_b*B-kp7NaD'QdWQPK"
    "Yq[@>P)hI;*_F]u`Rb[.j8_Q/"
    "<&>uu+VsH$sM9TA%?)(vmJ80),P7E>)tjD%2L=-t#fK[%`v=Q8<FfNkgg^oIbah*#8/"
    "Qt$F&:K*-(N/'+1vMB,u()-a.VUU*#[e%gAAO(S>WlA2);Sa"
    ">gXm8YB`1d@K#n]76-a$U,mF<fX]idqd)<3,]J7JmW4`6]uks=4-72L(jEk+:bJ0M^q-8Dm_Z?"
    "0olP1C9Sa&H[d&c$ooQUj]Exd*3ZM@-WGW2%s',B-_M%>%Ul:#/'xoFM9QX-$.QN'>"
    "[%$Z$uF6pA6Ki2O5:8w*vP1<-1`[G,)-m#>0`P&#eb#.3i)rtB61(o'$?X3B</"
    "R90;eZ]%Ncq;-Tl]#F>2Qft^ae_5tKL9MUe9b*sLEQ95C&`=G?@Mj=wh*'3E>=-<)Gt*Iw)'"
    "QG:`@I"
    "wOf7&]1i'S01B+Ev/Nac#9S;=;YQpg_6U`*kVY39xK,[/"
    "6Aj7:'1Bm-_1EYfa1+o&o4hp7KN_Q(OlIo@S%;jVdn0'1<Vc52=u`3^o-n1'g4v58Hj&6_t7$#"
    "#?M)c<$bgQ_'SY((-xkA#"
    "Y(,p'H9rIVY-b,'%bCPF7.J<Up^,(dU1VY*5#WkTU>h19w,WQhLI)3S#f$2(eb,jr*b;3Vw]*"
    "7NH%$c4Vs,eD9>XW8?N]o+(*pgC%/72LV-u<Hp,3@e^9UB1J+ak9-TN/mhKPg+AJYd$"
    "MlvAF_jCK*.O-^(63adMT->W%iewS8W6m2rtCpo'RS1R84=@paTKt)>=%&1[)*vp'u+x,VrwN;"
    "&]kuO9JDbg=pO$J*.jVe;u'm0dr9l,<*wMK*Oe=g8lV_KEBFkO'oU]^=[-792#ok,)"
    "i]lR8qQ2oA8wcRCZ^7w/Njh;?.stX?Q1>S1q4Bn$)K1<-rGdO'$Wr.Lc.CG)$/*JL4tNR/"
    ",SVO3,aUw'DJN:)Ss;wGn9A32ijw%FL+Z0Fn.U9;reSq)bmI32U==5ALuG&#Vf1398/pVo"
    "1*c-(aY168o<`JsSbk-,1N;$>0:OUas(3:8Z972LSfF8eb=c-;>SPw7.6hn3m`9^Xkn(r.qS["
    "0;T%&Qc=+STRxX'q1BNk3&*eu2;&8q$&x>Q#Q7^Tf+6<(d%ZVmj2bDi%.3L2n+4W'$P"
    "iDDG)g,r%+?,$@?uou5tSe2aN_AQU*<h`e-GI7)?OK2A.d7_c)?wQ5AS@DL3r#7fSkgl6-++D:"
    "'A,uq7SvlB$pcpH'q3n0#_%dY#xCpr-l<F0NR@-##FEV6NTF6##$l84N1w?AO>'IAO"
    "URQ##V^Fv-XFbGM7Fl(N<3DhLGF%q.1rC$#:T__&Pi68%0xi_&[qFJ(77j_&JWoF.V735&T,["
    "R*:xFR*K5>>#`bW-?4Ne_&6Ne_&6Ne_&n`kr-#GJcM6X;uM6X;uM(.a..^2TkL%oR(#"
    ";u.T%fAr%4tJ8&><1=GHZ_+m9/#H1F^R#SC#*N=BA9(D?v[UiFY>>^8p,KKF.W]L29uLkLlu/"
    "+4T<XoIB&hx=T1PcDaB&;HH+-AFr?(m9HZV)FKS8JCw;SD=6[^/DZUL`EUDf]GGlG&>"
    "w$)F./^n3+rlo+DB;5sIYGNk+i1t-69Jg--0pao7Sm#K)pdHW&;LuDNH@H>#/"
    "X-TI(;P>#,Gc>#0Su>#4`1?#8lC?#<xU?#@.i?#D:%@#HF7@#LRI@#P_[@#Tkn@#Xw*A#]-=A#"
    "a9OA#"
    "d<F&#*;G##.GY##2Sl##6`($#:l:$#>xL$#B.`$#F:r$#JF.%#NR@%#R_R%#Vke%#Zww%#_-4&"
    "#3^Rh%Sflr-k'MS.o?.5/sWel/wpEM0%3'/1)K^f1-d>G21&v(35>V`39V7A4=onx4"
    "A1OY5EI0;6Ibgr6M$HS7Q<)58C5w,;WoA*#[%T*#`1g*#d=#+#hI5+#lUG+#pbY+#tnl+#x$),"
    "#&1;,#*=M,#.I`,#2Ur,#6b.-#;w[H#iQtA#m^0B#qjBB#uvTB##-hB#'9$C#+E6C#"
    "/QHC#3^ZC#7jmC#;v)D#?,<D#C8ND#GDaD#KPsD#O]/"
    "E#g1A5#KA*1#gC17#MGd;#8(02#L-d3#rWM4#Hga1#,<w0#T.j<#O#'2#CYN1#qa^:#_4m3#o@"
    "/=#eG8=#t8J5#`+78#4uI-#"
    "m3B2#SB[8#Q0@8#i[*9#iOn8#1Nm;#^sN9#qh<9#:=x-#P;K2#$%X9#bC+.#Rg;<#mN=.#MTF."
    "#RZO.#2?)4#Y#(/#[)1/#b;L/#dAU/#0Sv;#lY$0#n`-0#sf60#(F24#wrH0#%/e0#"
    "TmD<#%JSMFove:CTBEXI:<eh2g)B,3h2^G3i;#d3jD>)4kMYD4lVu`4m`:&5niUA5@(A5BA1]"
    "PBB:xlBCC=2CDLXMCEUtiCf&0g2'tN?PGT4CPGT4CPGT4CPGT4CPGT4CPGT4CPGT4CP"
    "GT4CPGT4CPGT4CPGT4CPGT4CPGT4CP-qekC`.9kEg^+F$kwViFJTB&5KTB&5KTB&5KTB&5KTB&"
    "5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5o,^<-28ZI'O?;xp"
    "O?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xp;7q-#"
    "lLYI:xvD=#";

static const char* GetDefaultCompressedFontDataTTFBase85() {
  return proggy_clean_ttf_compressed_data_base85;
}

// Load embedded ProggyClean.ttf at size 13, disable oversampling
ImFont* AddFontDefault() {
  ImFontConfig font_cfg = ImFontConfig();
  // if (!font_cfg_template)
  {
    font_cfg.OversampleH = font_cfg.OversampleV = 1;
    font_cfg.PixelSnapH = true;
  }

  if (font_cfg.Name[0] == '\0') {
#ifdef WIN32
    strcpy_s(font_cfg.Name, SizeOfArray(font_cfg.Name),
             "ProggyClean.ttf, 13px");
#else
    strcpy(font_cfg.Name, "ProggyClean.ttf, 13px");
#endif
  }

  const char* ttf_compressed_base85 = GetDefaultCompressedFontDataTTFBase85();
  ImFont* font = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(
      ttf_compressed_base85, GParams.m_FontSize, &font_cfg,
      ImGui::GetIO().Fonts->GetGlyphRangesDefault());
  return font;
}

ImFont* AddOrbitFont(float pixel_size) {
  const auto exe_path = Path::GetExecutablePath();
  const auto font_file_name = exe_path + "fonts/Vera.ttf";
  return ImGui::GetIO().Fonts->AddFontFromFileTTF(font_file_name.c_str(),
                                                  pixel_size);
}

bool Orbit_ImGui_Init() {
  // g_Window = window;

  ImGuiIO& io = ImGui::GetIO();

  // http://doc.qt.io/qt-4.8/qt.html#Key-enum

  io.KeyMap[ImGuiKey_Tab] =
      0x00000001;  // Keyboard mapping. ImGui will use those indices to peek
                   // into the io.KeyDown[] array.
  io.KeyMap[ImGuiKey_LeftArrow] = 0x00000012;
  io.KeyMap[ImGuiKey_RightArrow] = 0x00000014;
  io.KeyMap[ImGuiKey_UpArrow] = 0x00000013;
  io.KeyMap[ImGuiKey_DownArrow] = 0x00000015;
  io.KeyMap[ImGuiKey_PageUp] = 0x00000016;
  io.KeyMap[ImGuiKey_PageDown] = 0x00000017;
  io.KeyMap[ImGuiKey_Home] = 0x00000010;
  io.KeyMap[ImGuiKey_End] = 0x00000011;
  io.KeyMap[ImGuiKey_Delete] = 0x00000007;
  io.KeyMap[ImGuiKey_Backspace] = 0x00000003;
  io.KeyMap[ImGuiKey_Enter] = 0x00000004;
  io.KeyMap[ImGuiKey_Escape] = 0x00000000;
  io.KeyMap[ImGuiKey_A] = 65;
  io.KeyMap[ImGuiKey_C] = 67;
  io.KeyMap[ImGuiKey_V] = 86;
  io.KeyMap[ImGuiKey_X] = 88;
  io.KeyMap[ImGuiKey_Y] = 89;
  io.KeyMap[ImGuiKey_Z] = 90;

  io.RenderDrawListsFn =
      Orbit_ImGui_RenderDrawLists;  // Alternatively you can set this to NULL
                                    // and call ImGui::GetDrawData() after
                                    // ImGui::Render() to get the same
                                    // ImDrawData pointer.
  /*io.SetClipboardTextFn = Orbit_ImGui_SetClipboardText;
  io.GetClipboardTextFn = Orbit_ImGui_GetClipboardText;*/

#ifdef _WIN32
  // io.ImeWindowHandle = glfwGetWin32Window(g_Window);
#endif

  SetupImGuiStyle(true, 1.f);

  const float kImguiFontOffset = 10.f;
  GOrbitImguiFont = AddOrbitFont(GParams.m_FontSize + kImguiFontOffset);
  ImGui::GetIO().Fonts->Build();

  return true;
}

void Orbit_ImGui_Shutdown() { Orbit_ImGui_InvalidateDeviceObjects(); }

void Orbit_ImGui_NewFrame(GlCanvas* a_Canvas) {
  if (!g_FontTexture) Orbit_ImGui_CreateDeviceObjects();

  ImGuiIO& io = ImGui::GetIO();

  static volatile bool doSsetTexture = true;
  if (doSsetTexture) {
    // SCOPE_TIMER_LOG( "glTexImage2D" );
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);
    glBindTexture(GL_TEXTURE_2D, g_FontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA,
                 GL_UNSIGNED_BYTE, pixels);

    glBindTexture(GL_TEXTURE_2D, GTextureInjected);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, inject_image.width,
                 inject_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 inject_image.pixel_data);

    glBindTexture(GL_TEXTURE_2D, GTextureTimer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, timer_image.width,
                 timer_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 timer_image.pixel_data);

    glBindTexture(GL_TEXTURE_2D, GTextureHelp);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, help_image.width, help_image.height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, help_image.pixel_data);

    glBindTexture(GL_TEXTURE_2D, GTextureRecord);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, record_image.width,
                 record_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 record_image.pixel_data);
  }

  // Store our identifier
  io.Fonts->TexID =
      reinterpret_cast<void*>(static_cast<intptr_t>(g_FontTexture));

  // Setup display size (every frame to accommodate for window resizing)
  int w = a_Canvas->getWidth();
  int h = a_Canvas->getHeight();
  // int display_w;
  // int display_h;
  io.DisplaySize = ImVec2(w, h);
  // io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h
  // / h);

  // Setup time step
  // TODO: Does this make sense?
  if (a_Canvas->GetDeltaTimeSeconds() > 0) {
    io.DeltaTime = a_Canvas->GetDeltaTimeSeconds();
  }

  // Setup inputs
  io.MousePos = ImVec2(
      a_Canvas->GetMousePosX(),
      a_Canvas
          ->GetMousePosY());  // Mouse position in screen coordinates (set to
                              // -1,-1 if no mouse / on another screen, etc.)

  for (int i = 0; i < 3; i++) {
    io.MouseDown[i] = g_MousePressed[i];
    // g_MousePressed[i] = false;
  }

  io.MouseWheel = g_MouseWheel;
  g_MouseWheel = 0.0f;

  // Start the frame
  ImGui::NewFrame();
}

//-----------------------------------------------------------------------------
void WatchWindow::Draw(const char* title, bool* p_opened) {
  ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin(title, p_opened)) {
    ImGui::End();
    return;
  }

  const std::vector<std::shared_ptr<Variable> >& watchedVars =
      Capture::GTargetProcess->GetWatchedVariables();

  if (ImGui::Button("Sync")) {
    for (std::shared_ptr<Variable> var : watchedVars) {
      var->SyncValue();
    }
  }

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
  ImGui::Columns(2);
  ImGui::Separator();

  // struct funcs
  {
    for (std::shared_ptr<Variable> variable : watchedVars) {
      Variable& var = *variable;
      ImGui::PushID(
          variable.get());  // Use object uid as identifier. Most commonly you
                            // could also use the object pointer as a base ID.
      ImGui::AlignFirstTextHeightToWidgets();  // Text and Tree nodes are less
                                               // high than regular widgets,
                                               // here we add vertical spacing
                                               // to make the tree lines equal
                                               // high.
      ImGui::Text("%s", var.m_Name.c_str());
      ImGui::NextColumn();
      ImGui::AlignFirstTextHeightToWidgets();

      switch (var.m_BasicType) {
        case Variable::Int: {
          int val = var.m_Int;
          ImGui::DragInt("##value", &var.m_Int);
          var.SetValue(val);
          break;
        }
        // case Variable::Int64:
        // case Variable::UInt64:
        case Variable::UInt: {
          int val = var.m_Int;
          ImGui::DragInt("##value", &var.m_Int, 1.f, 0, INT_MAX);
          var.SetValue(val);
          break;
        }
        case Variable::Float: {
          float val = var.m_Float;
          ImGui::DragFloat("##value", &var.m_Float, 0.01f);
          var.SetValue(val);
          break;
        }
        // case Variable::Double:
        case Variable::Bool: {
          bool val = var.m_Bool;
          ImGui::Checkbox("##value", &var.m_Bool);
          var.SetValue(val);
          break;
        }
        default:
          ImGui::Text("Unknown Type");
      }

      ImGui::NextColumn();

      ImGui::PopID();
    }
  };

  ImGui::Columns(1);
  ImGui::Separator();
  ImGui::PopStyleVar();
  ImGui::End();
}

// From Avoid/Doug Binks
void SetupImGuiStyle(bool bStyleDark_, float alpha_) {
  ImGuiStyle& style = ImGui::GetStyle();

  // light style from Pacome Danhiez (user itamago)
  // https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
  style.Alpha = 1.0f;
  style.FrameRounding = 3.0f;
  style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
  style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
  style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  // style.Colors[ImGuiCol_PopupBg] = ImVec4( 1.00f, 1.00f, 1.00f, 0.94f );
  style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.19f);
  style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
  style.Colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
  style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
  style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  style.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
  style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
  style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
  style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
  style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
  style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
  style.Colors[ImGuiCol_ScrollbarGrabHovered] =
      ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
  style.Colors[ImGuiCol_ScrollbarGrabActive] =
      ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
  // style.Colors[ImGuiCol_ComboBg] = ImVec4( 0.86f, 0.86f, 0.86f, 0.99f );
  style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
  style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
  style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
  style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
  style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_Separator] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
  style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
  style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
  style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
  // style.Colors[ImGuiCol_CloseButton] = ImVec4( 0.59f, 0.59f, 0.59f, 0.50f );
  // style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4( 0.98f, 0.39f,
  // 0.36f, 1.00f ); style.Colors[ImGuiCol_CloseButtonActive] = ImVec4( 0.98f,
  // 0.39f, 0.36f, 1.00f );
  style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
  style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_PlotHistogramHovered] =
      ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
  style.Colors[ImGuiCol_ModalWindowDarkening] =
      ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

  if (bStyleDark_) {
    for (int i = 0; i <= ImGuiCol_COUNT; i++) {
      ImVec4& col = style.Colors[i];
      float H, S, V;
      ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);

      if (S < 0.1f) {
        V = 1.0f - V;
      }
      ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
      if (col.w < 1.00f) {
        col.w *= alpha_;
      }
    }
  } else {
    for (int i = 0; i <= ImGuiCol_COUNT; i++) {
      ImVec4& col = style.Colors[i];
      if (col.w < 1.00f) {
        col.x *= alpha_;
        col.y *= alpha_;
        col.z *= alpha_;
        col.w *= alpha_;
      }
    }
  }
}

void OutputWindow::AddLine(const std::string& a_String) {
  int old_size = Buf.size();
  Buf.append(a_String.c_str());
  Buf.append("\n");
  for (int new_size = Buf.size(); old_size < new_size; old_size++)
    if (Buf[old_size] == '\n') LineOffsets.push_back(old_size);
}

void OutputWindow::Draw(const char* title, bool* p_opened, ImVec2* a_Size) {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

  if (a_Size) {
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImVec2 CanvasSize = *a_Size;
    CanvasSize.x -= 20;
    CanvasSize.y -= 20;
    ImGui::SetNextWindowSize(CanvasSize, ImGuiCond_Always);
    ImGui::Begin(title, p_opened, CanvasSize, 1.f, WindowFlags);
  } else {
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImVec2 size(400, 400);
    ImGui::Begin(title, p_opened, size, 1.f, WindowFlags);
  }

  ImGui::TextUnformatted(Buf.begin());

  static bool checked = true;
  ImGui::Checkbox("blah", &checked);

  static int i1 = 0;
  if (ImGui::SliderInt("slider int", &i1, 10, 100)) {
    GCurrentTimeGraph->SetFontSize(i1);
  }

  ImGui::End();
  ImGui::PopStyleVar();
}
