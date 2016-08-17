#include "Precomp.h"

static HWND WindowInit(HINSTANCE instance, const wchar_t* class_name, int32_t width, int32_t height);
static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static bool LoadScene(IRZRenderer* renderer);

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int show_command)
{
  HWND window = WindowInit(instance, L"RZRenderersTest", 1280, 960);
  if (!window)
  {
    return -1;
  }

  IRZRenderer* renderer = nullptr;
  RZRendererCreateParams params{};
  params.WindowHandle = window;
  params.RenderWidth = 640;
  params.RenderHeight = 480;
  params.HorizFOV = 60.f * (3.14156f / 180.f);

  if (!RZRendererCreate(RZRenderer_CPURaytracer, &params, &renderer))
  {
    return -2;
  }

  ShowWindow(window, show_command);
  UpdateWindow(window);

  if (!LoadScene(renderer))
  {
    renderer->Release();
    return -3;
  }

  RZVector3 position{ 0.f, 25.f, -200.f };
  //RZVector3 position{ 0.f, 0.f, -1.5f };
  RZQuaternion orientation{ 0.f, 0.f, 0.f, 1.f };

  MSG msg = { 0 };
  while (msg.message != WM_QUIT)
  {
    if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    else
    {
      if (GetAsyncKeyState(VK_LEFT) & 0x8000)
        position.x -= 0.125f;
      if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
        position.x += 0.125f;
      if (GetAsyncKeyState(VK_DOWN) & 0x8000)
        position.z -= 0.125f;
      if (GetAsyncKeyState(VK_UP) & 0x8000)
        position.z += 0.125f;
      renderer->RenderScene(position, orientation);
    }
  }

  renderer->Release();
  DestroyWindow(window);
  return 0;
}

HWND WindowInit(HINSTANCE instance, const wchar_t* class_name, int32_t width, int32_t height)
{
  WNDCLASSEX wcx = { 0 };
  wcx.cbSize = sizeof(wcx);
  wcx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wcx.hInstance = instance;
  wcx.lpfnWndProc = WindowProc;
  wcx.lpszClassName = class_name;
  if (RegisterClassEx(&wcx) == INVALID_ATOM)
  {
    assert(false);
    return NULL;
  }

  DWORD style = WS_OVERLAPPEDWINDOW;

  RECT rc = { 0 };
  rc.right = width;
  rc.bottom = height;
  AdjustWindowRect(&rc, style, FALSE);

  HWND hwnd = CreateWindow(wcx.lpszClassName, wcx.lpszClassName, style, CW_USEDEFAULT, CW_USEDEFAULT,
    rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, instance, NULL);
  if (!hwnd)
  {
    assert(false);
    return NULL;
  }

  return hwnd;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_CLOSE:
    PostQuitMessage(0);
    break;

  case WM_KEYDOWN:
    if (wParam == VK_ESCAPE)
    {
      PostQuitMessage(0);
    }
    break;
  }

  return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool LoadScene(IRZRenderer* renderer)
{
  UNREFERENCED_PARAMETER(renderer);

  Assimp::Importer imp;
  const aiScene* scene = imp.ReadFile("C:\\src\\assets\\teapot\\teapot.obj",
  //const aiScene* scene = imp.ReadFile("C:\\src\\assets\\buddha\\buddha.obj",
    aiProcess_GenSmoothNormals | aiProcess_Triangulate |
    aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes |
    aiProcess_JoinIdenticalVertices);
  if (!scene)
  {
    return false;
  }

  std::vector<uint32_t> indices(1024);
  for (uint32_t i_mesh = 0; i_mesh < scene->mNumMeshes; ++i_mesh)
  {
    const aiMesh* mesh = scene->mMeshes[i_mesh];

    uint32_t base_index = renderer->AddVertices(mesh->mNumVertices, (const RZVector3*)mesh->mVertices);

    indices.clear();

    for (uint32_t i_face = 0; i_face < mesh->mNumFaces; ++i_face)
    {
      const aiFace* face = &mesh->mFaces[i_face];
      assert(face->mNumIndices == 3);
      indices.push_back(base_index + face->mIndices[0]);
      indices.push_back(base_index + face->mIndices[1]);
      indices.push_back(base_index + face->mIndices[2]);
    }

    renderer->AddMesh((uint32_t)indices.size(), indices.data());
  }

  return true;
}
