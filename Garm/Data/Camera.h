#ifndef DATA_CAMERA_H
#define DATA_CAMERA_H

#include <DirectXMath.h>

using DirectX::XMVECTOR;

namespace Garm
{
	namespace Data
	{
		class Camera {
		public:
			Camera();
			virtual ~Camera();

			void Update( XMVECTOR location );
			void Update( XMVECTOR location, XMVECTOR lookAt );
			void Update( XMVECTOR location, float azimuth, float altitude = -0.5f );

			XMVECTOR GetLocation();
			XMVECTOR GetLookAt();
		private:
			XMVECTOR location;
			XMVECTOR lookAt;

		};
	}
}

#endif