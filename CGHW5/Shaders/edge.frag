// Image texture.
uniform sampler2D ImageTexture;
varying vec2 vv2texture;
varying vec4 vv4color;
uniform float shaderParam;

void main()
{
  // Width of Image.
  float fWidth = 500.0;
  // Height of Image.
  float fHeight = 500.0;
  // X directional search matrix.
  mat3 GX = mat3( -1.0, 0.0, 1.0,
                 -2.0, 0.0, 2.0,
                 -1.0, 0.0, 1.0 );
  // Y directional search matrix.
  mat3 GY =  mat3( 1.0,  2.0,  1.0,
                  0.0,  0.0,  0.0,
                  -1.0, -2.0, -1.0 );
  
  vec4  fSumX = vec4( 0.0,0.0,0.0,0.0 );
  vec4  fSumY = vec4( 0.0,0.0,0.0,0.0 );
  vec4 fTotalSum = vec4( 0.0,0.0,0.0,0.0 );
  
  // Findout X , Y index of incoming pixel
  // from its texture coordinate.
  float fXIndex = vv2texture.s * fWidth;
  float fYIndex = vv2texture.t * fHeight;
  
  /* image boundaries Top, Bottom, Left, Right pixels*/
  if( ! ( fYIndex < 1.0 || fYIndex > fHeight - 1.0 ||
         fXIndex < 1.0 || fXIndex > fWidth - 1.0 ))
  {
    // X Directional Gradient calculation.
    for(float I=-1.0; I<=1.0; I = I + 1.0)
    {
      for(float J=-1.0; J<=1.0; J = J + 1.0)
      {
        float fTempX = ( fXIndex + I + 0.5 ) / fWidth ;
        float fTempY = ( fYIndex + J + 0.5 ) / fHeight ;
        vec4 fTempSumX = texture2D( ImageTexture, vec2( fTempX, fTempY ));
        fSumX = fSumX + ( fTempSumX * vec4( GX[int(I+1.0)][int(J+1.0)],
                                           GX[int(I+1.0)][int(J+1.0)],
                                           GX[int(I+1.0)][int(J+1.0)],
                                           GX[int(I+1.0)][int(J+1.0)]));
      }
    }
    
    { // Y Directional Gradient calculation.
      for(float I=-1.0; I<=1.0; I = I + 1.0)
      {
        for(float J=-1.0; J<=1.0; J = J + 1.0)
        {
          float fTempX = ( fXIndex + I + 0.5 ) / fWidth ;
          float fTempY = ( fYIndex + J + 0.5 ) / fHeight ;
          vec4 fTempSumY = texture2D( ImageTexture, vec2( fTempX, fTempY ));
          fSumY = fSumY + ( fTempSumY * vec4( GY[int(I+1.0)][int(J+1.0)],
                                             GY[int(I+1.0)][int(J+1.0)],
                                             GY[int(I+1.0)][int(J+1.0)],
                                             GY[int(I+1.0)][int(J+1.0)]));
        }
      }
      // Combine X Directional and Y Directional Gradient.
      vec4 fTem = fSumX * fSumX + fSumY * fSumY;
      fTotalSum = sqrt( fTem );
    }
  }
  // Creating displayable edge data.
  fTotalSum = vec4( 1.0,1.0,1.0,1.0) - fTotalSum;
  
  gl_FragColor = fTotalSum * vv4color;
}