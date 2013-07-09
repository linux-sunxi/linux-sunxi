/*
 *************************************************************************
 * Ralink Tech Inc.
 * 5F., No.36, Taiyuan St., Jhubei City,
 * Hsinchu County 302,
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2010, Ralink Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program; if not, write to the                         *
 * Free Software Foundation, Inc.,                                       *
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                                                                       *
 *************************************************************************/


#include "crypt_aes.h"


#define assert(a)   /*ASSERT(a)
*/

#undef u32
#define	u32 unsigned int
/*typedef unsigned int u32;
*/

static const u32 Te0[256] = {
    0xc66363a5U, 0xf87c7c84U, 0xee777799U, 0xf67b7b8dU,
    0xfff2f20dU, 0xd66b6bbdU, 0xde6f6fb1U, 0x91c5c554U,
    0x60303050U, 0x02010103U, 0xce6767a9U, 0x562b2b7dU,
    0xe7fefe19U, 0xb5d7d762U, 0x4dababe6U, 0xec76769aU,
    0x8fcaca45U, 0x1f82829dU, 0x89c9c940U, 0xfa7d7d87U,
    0xeffafa15U, 0xb25959ebU, 0x8e4747c9U, 0xfbf0f00bU,
    0x41adadecU, 0xb3d4d467U, 0x5fa2a2fdU, 0x45afafeaU,
    0x239c9cbfU, 0x53a4a4f7U, 0xe4727296U, 0x9bc0c05bU,
    0x75b7b7c2U, 0xe1fdfd1cU, 0x3d9393aeU, 0x4c26266aU,
    0x6c36365aU, 0x7e3f3f41U, 0xf5f7f702U, 0x83cccc4fU,
    0x6834345cU, 0x51a5a5f4U, 0xd1e5e534U, 0xf9f1f108U,
    0xe2717193U, 0xabd8d873U, 0x62313153U, 0x2a15153fU,
    0x0804040cU, 0x95c7c752U, 0x46232365U, 0x9dc3c35eU,
    0x30181828U, 0x379696a1U, 0x0a05050fU, 0x2f9a9ab5U,
    0x0e070709U, 0x24121236U, 0x1b80809bU, 0xdfe2e23dU,
    0xcdebeb26U, 0x4e272769U, 0x7fb2b2cdU, 0xea75759fU,
    0x1209091bU, 0x1d83839eU, 0x582c2c74U, 0x341a1a2eU,
    0x361b1b2dU, 0xdc6e6eb2U, 0xb45a5aeeU, 0x5ba0a0fbU,
    0xa45252f6U, 0x763b3b4dU, 0xb7d6d661U, 0x7db3b3ceU,
    0x5229297bU, 0xdde3e33eU, 0x5e2f2f71U, 0x13848497U,
    0xa65353f5U, 0xb9d1d168U, 0x00000000U, 0xc1eded2cU,
    0x40202060U, 0xe3fcfc1fU, 0x79b1b1c8U, 0xb65b5bedU,
    0xd46a6abeU, 0x8dcbcb46U, 0x67bebed9U, 0x7239394bU,
    0x944a4adeU, 0x984c4cd4U, 0xb05858e8U, 0x85cfcf4aU,
    0xbbd0d06bU, 0xc5efef2aU, 0x4faaaae5U, 0xedfbfb16U,
    0x864343c5U, 0x9a4d4dd7U, 0x66333355U, 0x11858594U,
    0x8a4545cfU, 0xe9f9f910U, 0x04020206U, 0xfe7f7f81U,
    0xa05050f0U, 0x783c3c44U, 0x259f9fbaU, 0x4ba8a8e3U,
    0xa25151f3U, 0x5da3a3feU, 0x804040c0U, 0x058f8f8aU,
    0x3f9292adU, 0x219d9dbcU, 0x70383848U, 0xf1f5f504U,
    0x63bcbcdfU, 0x77b6b6c1U, 0xafdada75U, 0x42212163U,
    0x20101030U, 0xe5ffff1aU, 0xfdf3f30eU, 0xbfd2d26dU,
    0x81cdcd4cU, 0x180c0c14U, 0x26131335U, 0xc3ecec2fU,
    0xbe5f5fe1U, 0x359797a2U, 0x884444ccU, 0x2e171739U,
    0x93c4c457U, 0x55a7a7f2U, 0xfc7e7e82U, 0x7a3d3d47U,
    0xc86464acU, 0xba5d5de7U, 0x3219192bU, 0xe6737395U,
    0xc06060a0U, 0x19818198U, 0x9e4f4fd1U, 0xa3dcdc7fU,
    0x44222266U, 0x542a2a7eU, 0x3b9090abU, 0x0b888883U,
    0x8c4646caU, 0xc7eeee29U, 0x6bb8b8d3U, 0x2814143cU,
    0xa7dede79U, 0xbc5e5ee2U, 0x160b0b1dU, 0xaddbdb76U,
    0xdbe0e03bU, 0x64323256U, 0x743a3a4eU, 0x140a0a1eU,
    0x924949dbU, 0x0c06060aU, 0x4824246cU, 0xb85c5ce4U,
    0x9fc2c25dU, 0xbdd3d36eU, 0x43acacefU, 0xc46262a6U,
    0x399191a8U, 0x319595a4U, 0xd3e4e437U, 0xf279798bU,
    0xd5e7e732U, 0x8bc8c843U, 0x6e373759U, 0xda6d6db7U,
    0x018d8d8cU, 0xb1d5d564U, 0x9c4e4ed2U, 0x49a9a9e0U,
    0xd86c6cb4U, 0xac5656faU, 0xf3f4f407U, 0xcfeaea25U,
    0xca6565afU, 0xf47a7a8eU, 0x47aeaee9U, 0x10080818U,
    0x6fbabad5U, 0xf0787888U, 0x4a25256fU, 0x5c2e2e72U,
    0x381c1c24U, 0x57a6a6f1U, 0x73b4b4c7U, 0x97c6c651U,
    0xcbe8e823U, 0xa1dddd7cU, 0xe874749cU, 0x3e1f1f21U,
    0x964b4bddU, 0x61bdbddcU, 0x0d8b8b86U, 0x0f8a8a85U,
    0xe0707090U, 0x7c3e3e42U, 0x71b5b5c4U, 0xcc6666aaU,
    0x904848d8U, 0x06030305U, 0xf7f6f601U, 0x1c0e0e12U,
    0xc26161a3U, 0x6a35355fU, 0xae5757f9U, 0x69b9b9d0U,
    0x17868691U, 0x99c1c158U, 0x3a1d1d27U, 0x279e9eb9U,
    0xd9e1e138U, 0xebf8f813U, 0x2b9898b3U, 0x22111133U,
    0xd26969bbU, 0xa9d9d970U, 0x078e8e89U, 0x339494a7U,
    0x2d9b9bb6U, 0x3c1e1e22U, 0x15878792U, 0xc9e9e920U,
    0x87cece49U, 0xaa5555ffU, 0x50282878U, 0xa5dfdf7aU,
    0x038c8c8fU, 0x59a1a1f8U, 0x09898980U, 0x1a0d0d17U,
    0x65bfbfdaU, 0xd7e6e631U, 0x844242c6U, 0xd06868b8U,
    0x824141c3U, 0x299999b0U, 0x5a2d2d77U, 0x1e0f0f11U,
    0x7bb0b0cbU, 0xa85454fcU, 0x6dbbbbd6U, 0x2c16163aU,
};
static const u32 Te1[256] = {
    0xa5c66363U, 0x84f87c7cU, 0x99ee7777U, 0x8df67b7bU,
    0x0dfff2f2U, 0xbdd66b6bU, 0xb1de6f6fU, 0x5491c5c5U,
    0x50603030U, 0x03020101U, 0xa9ce6767U, 0x7d562b2bU,
    0x19e7fefeU, 0x62b5d7d7U, 0xe64dababU, 0x9aec7676U,
    0x458fcacaU, 0x9d1f8282U, 0x4089c9c9U, 0x87fa7d7dU,
    0x15effafaU, 0xebb25959U, 0xc98e4747U, 0x0bfbf0f0U,
    0xec41adadU, 0x67b3d4d4U, 0xfd5fa2a2U, 0xea45afafU,
    0xbf239c9cU, 0xf753a4a4U, 0x96e47272U, 0x5b9bc0c0U,
    0xc275b7b7U, 0x1ce1fdfdU, 0xae3d9393U, 0x6a4c2626U,
    0x5a6c3636U, 0x417e3f3fU, 0x02f5f7f7U, 0x4f83ccccU,
    0x5c683434U, 0xf451a5a5U, 0x34d1e5e5U, 0x08f9f1f1U,
    0x93e27171U, 0x73abd8d8U, 0x53623131U, 0x3f2a1515U,
    0x0c080404U, 0x5295c7c7U, 0x65462323U, 0x5e9dc3c3U,
    0x28301818U, 0xa1379696U, 0x0f0a0505U, 0xb52f9a9aU,
    0x090e0707U, 0x36241212U, 0x9b1b8080U, 0x3ddfe2e2U,
    0x26cdebebU, 0x694e2727U, 0xcd7fb2b2U, 0x9fea7575U,
    0x1b120909U, 0x9e1d8383U, 0x74582c2cU, 0x2e341a1aU,
    0x2d361b1bU, 0xb2dc6e6eU, 0xeeb45a5aU, 0xfb5ba0a0U,
    0xf6a45252U, 0x4d763b3bU, 0x61b7d6d6U, 0xce7db3b3U,
    0x7b522929U, 0x3edde3e3U, 0x715e2f2fU, 0x97138484U,
    0xf5a65353U, 0x68b9d1d1U, 0x00000000U, 0x2cc1ededU,
    0x60402020U, 0x1fe3fcfcU, 0xc879b1b1U, 0xedb65b5bU,
    0xbed46a6aU, 0x468dcbcbU, 0xd967bebeU, 0x4b723939U,
    0xde944a4aU, 0xd4984c4cU, 0xe8b05858U, 0x4a85cfcfU,
    0x6bbbd0d0U, 0x2ac5efefU, 0xe54faaaaU, 0x16edfbfbU,
    0xc5864343U, 0xd79a4d4dU, 0x55663333U, 0x94118585U,
    0xcf8a4545U, 0x10e9f9f9U, 0x06040202U, 0x81fe7f7fU,
    0xf0a05050U, 0x44783c3cU, 0xba259f9fU, 0xe34ba8a8U,
    0xf3a25151U, 0xfe5da3a3U, 0xc0804040U, 0x8a058f8fU,
    0xad3f9292U, 0xbc219d9dU, 0x48703838U, 0x04f1f5f5U,
    0xdf63bcbcU, 0xc177b6b6U, 0x75afdadaU, 0x63422121U,
    0x30201010U, 0x1ae5ffffU, 0x0efdf3f3U, 0x6dbfd2d2U,
    0x4c81cdcdU, 0x14180c0cU, 0x35261313U, 0x2fc3ececU,
    0xe1be5f5fU, 0xa2359797U, 0xcc884444U, 0x392e1717U,
    0x5793c4c4U, 0xf255a7a7U, 0x82fc7e7eU, 0x477a3d3dU,
    0xacc86464U, 0xe7ba5d5dU, 0x2b321919U, 0x95e67373U,
    0xa0c06060U, 0x98198181U, 0xd19e4f4fU, 0x7fa3dcdcU,
    0x66442222U, 0x7e542a2aU, 0xab3b9090U, 0x830b8888U,
    0xca8c4646U, 0x29c7eeeeU, 0xd36bb8b8U, 0x3c281414U,
    0x79a7dedeU, 0xe2bc5e5eU, 0x1d160b0bU, 0x76addbdbU,
    0x3bdbe0e0U, 0x56643232U, 0x4e743a3aU, 0x1e140a0aU,
    0xdb924949U, 0x0a0c0606U, 0x6c482424U, 0xe4b85c5cU,
    0x5d9fc2c2U, 0x6ebdd3d3U, 0xef43acacU, 0xa6c46262U,
    0xa8399191U, 0xa4319595U, 0x37d3e4e4U, 0x8bf27979U,
    0x32d5e7e7U, 0x438bc8c8U, 0x596e3737U, 0xb7da6d6dU,
    0x8c018d8dU, 0x64b1d5d5U, 0xd29c4e4eU, 0xe049a9a9U,
    0xb4d86c6cU, 0xfaac5656U, 0x07f3f4f4U, 0x25cfeaeaU,
    0xafca6565U, 0x8ef47a7aU, 0xe947aeaeU, 0x18100808U,
    0xd56fbabaU, 0x88f07878U, 0x6f4a2525U, 0x725c2e2eU,
    0x24381c1cU, 0xf157a6a6U, 0xc773b4b4U, 0x5197c6c6U,
    0x23cbe8e8U, 0x7ca1ddddU, 0x9ce87474U, 0x213e1f1fU,
    0xdd964b4bU, 0xdc61bdbdU, 0x860d8b8bU, 0x850f8a8aU,
    0x90e07070U, 0x427c3e3eU, 0xc471b5b5U, 0xaacc6666U,
    0xd8904848U, 0x05060303U, 0x01f7f6f6U, 0x121c0e0eU,
    0xa3c26161U, 0x5f6a3535U, 0xf9ae5757U, 0xd069b9b9U,
    0x91178686U, 0x5899c1c1U, 0x273a1d1dU, 0xb9279e9eU,
    0x38d9e1e1U, 0x13ebf8f8U, 0xb32b9898U, 0x33221111U,
    0xbbd26969U, 0x70a9d9d9U, 0x89078e8eU, 0xa7339494U,
    0xb62d9b9bU, 0x223c1e1eU, 0x92158787U, 0x20c9e9e9U,
    0x4987ceceU, 0xffaa5555U, 0x78502828U, 0x7aa5dfdfU,
    0x8f038c8cU, 0xf859a1a1U, 0x80098989U, 0x171a0d0dU,
    0xda65bfbfU, 0x31d7e6e6U, 0xc6844242U, 0xb8d06868U,
    0xc3824141U, 0xb0299999U, 0x775a2d2dU, 0x111e0f0fU,
    0xcb7bb0b0U, 0xfca85454U, 0xd66dbbbbU, 0x3a2c1616U,
};
static const u32 Te2[256] = {
    0x63a5c663U, 0x7c84f87cU, 0x7799ee77U, 0x7b8df67bU,
    0xf20dfff2U, 0x6bbdd66bU, 0x6fb1de6fU, 0xc55491c5U,
    0x30506030U, 0x01030201U, 0x67a9ce67U, 0x2b7d562bU,
    0xfe19e7feU, 0xd762b5d7U, 0xabe64dabU, 0x769aec76U,
    0xca458fcaU, 0x829d1f82U, 0xc94089c9U, 0x7d87fa7dU,
    0xfa15effaU, 0x59ebb259U, 0x47c98e47U, 0xf00bfbf0U,
    0xadec41adU, 0xd467b3d4U, 0xa2fd5fa2U, 0xafea45afU,
    0x9cbf239cU, 0xa4f753a4U, 0x7296e472U, 0xc05b9bc0U,
    0xb7c275b7U, 0xfd1ce1fdU, 0x93ae3d93U, 0x266a4c26U,
    0x365a6c36U, 0x3f417e3fU, 0xf702f5f7U, 0xcc4f83ccU,
    0x345c6834U, 0xa5f451a5U, 0xe534d1e5U, 0xf108f9f1U,
    0x7193e271U, 0xd873abd8U, 0x31536231U, 0x153f2a15U,
    0x040c0804U, 0xc75295c7U, 0x23654623U, 0xc35e9dc3U,
    0x18283018U, 0x96a13796U, 0x050f0a05U, 0x9ab52f9aU,
    0x07090e07U, 0x12362412U, 0x809b1b80U, 0xe23ddfe2U,
    0xeb26cdebU, 0x27694e27U, 0xb2cd7fb2U, 0x759fea75U,
    0x091b1209U, 0x839e1d83U, 0x2c74582cU, 0x1a2e341aU,
    0x1b2d361bU, 0x6eb2dc6eU, 0x5aeeb45aU, 0xa0fb5ba0U,
    0x52f6a452U, 0x3b4d763bU, 0xd661b7d6U, 0xb3ce7db3U,
    0x297b5229U, 0xe33edde3U, 0x2f715e2fU, 0x84971384U,
    0x53f5a653U, 0xd168b9d1U, 0x00000000U, 0xed2cc1edU,
    0x20604020U, 0xfc1fe3fcU, 0xb1c879b1U, 0x5bedb65bU,
    0x6abed46aU, 0xcb468dcbU, 0xbed967beU, 0x394b7239U,
    0x4ade944aU, 0x4cd4984cU, 0x58e8b058U, 0xcf4a85cfU,
    0xd06bbbd0U, 0xef2ac5efU, 0xaae54faaU, 0xfb16edfbU,
    0x43c58643U, 0x4dd79a4dU, 0x33556633U, 0x85941185U,
    0x45cf8a45U, 0xf910e9f9U, 0x02060402U, 0x7f81fe7fU,
    0x50f0a050U, 0x3c44783cU, 0x9fba259fU, 0xa8e34ba8U,
    0x51f3a251U, 0xa3fe5da3U, 0x40c08040U, 0x8f8a058fU,
    0x92ad3f92U, 0x9dbc219dU, 0x38487038U, 0xf504f1f5U,
    0xbcdf63bcU, 0xb6c177b6U, 0xda75afdaU, 0x21634221U,
    0x10302010U, 0xff1ae5ffU, 0xf30efdf3U, 0xd26dbfd2U,
    0xcd4c81cdU, 0x0c14180cU, 0x13352613U, 0xec2fc3ecU,
    0x5fe1be5fU, 0x97a23597U, 0x44cc8844U, 0x17392e17U,
    0xc45793c4U, 0xa7f255a7U, 0x7e82fc7eU, 0x3d477a3dU,
    0x64acc864U, 0x5de7ba5dU, 0x192b3219U, 0x7395e673U,
    0x60a0c060U, 0x81981981U, 0x4fd19e4fU, 0xdc7fa3dcU,
    0x22664422U, 0x2a7e542aU, 0x90ab3b90U, 0x88830b88U,
    0x46ca8c46U, 0xee29c7eeU, 0xb8d36bb8U, 0x143c2814U,
    0xde79a7deU, 0x5ee2bc5eU, 0x0b1d160bU, 0xdb76addbU,
    0xe03bdbe0U, 0x32566432U, 0x3a4e743aU, 0x0a1e140aU,
    0x49db9249U, 0x060a0c06U, 0x246c4824U, 0x5ce4b85cU,
    0xc25d9fc2U, 0xd36ebdd3U, 0xacef43acU, 0x62a6c462U,
    0x91a83991U, 0x95a43195U, 0xe437d3e4U, 0x798bf279U,
    0xe732d5e7U, 0xc8438bc8U, 0x37596e37U, 0x6db7da6dU,
    0x8d8c018dU, 0xd564b1d5U, 0x4ed29c4eU, 0xa9e049a9U,
    0x6cb4d86cU, 0x56faac56U, 0xf407f3f4U, 0xea25cfeaU,
    0x65afca65U, 0x7a8ef47aU, 0xaee947aeU, 0x08181008U,
    0xbad56fbaU, 0x7888f078U, 0x256f4a25U, 0x2e725c2eU,
    0x1c24381cU, 0xa6f157a6U, 0xb4c773b4U, 0xc65197c6U,
    0xe823cbe8U, 0xdd7ca1ddU, 0x749ce874U, 0x1f213e1fU,
    0x4bdd964bU, 0xbddc61bdU, 0x8b860d8bU, 0x8a850f8aU,
    0x7090e070U, 0x3e427c3eU, 0xb5c471b5U, 0x66aacc66U,
    0x48d89048U, 0x03050603U, 0xf601f7f6U, 0x0e121c0eU,
    0x61a3c261U, 0x355f6a35U, 0x57f9ae57U, 0xb9d069b9U,
    0x86911786U, 0xc15899c1U, 0x1d273a1dU, 0x9eb9279eU,
    0xe138d9e1U, 0xf813ebf8U, 0x98b32b98U, 0x11332211U,
    0x69bbd269U, 0xd970a9d9U, 0x8e89078eU, 0x94a73394U,
    0x9bb62d9bU, 0x1e223c1eU, 0x87921587U, 0xe920c9e9U,
    0xce4987ceU, 0x55ffaa55U, 0x28785028U, 0xdf7aa5dfU,
    0x8c8f038cU, 0xa1f859a1U, 0x89800989U, 0x0d171a0dU,
    0xbfda65bfU, 0xe631d7e6U, 0x42c68442U, 0x68b8d068U,
    0x41c38241U, 0x99b02999U, 0x2d775a2dU, 0x0f111e0fU,
    0xb0cb7bb0U, 0x54fca854U, 0xbbd66dbbU, 0x163a2c16U,
};
static const u32 Te3[256] = {
    0x6363a5c6U, 0x7c7c84f8U, 0x777799eeU, 0x7b7b8df6U,
    0xf2f20dffU, 0x6b6bbdd6U, 0x6f6fb1deU, 0xc5c55491U,
    0x30305060U, 0x01010302U, 0x6767a9ceU, 0x2b2b7d56U,
    0xfefe19e7U, 0xd7d762b5U, 0xababe64dU, 0x76769aecU,
    0xcaca458fU, 0x82829d1fU, 0xc9c94089U, 0x7d7d87faU,
    0xfafa15efU, 0x5959ebb2U, 0x4747c98eU, 0xf0f00bfbU,
    0xadadec41U, 0xd4d467b3U, 0xa2a2fd5fU, 0xafafea45U,
    0x9c9cbf23U, 0xa4a4f753U, 0x727296e4U, 0xc0c05b9bU,
    0xb7b7c275U, 0xfdfd1ce1U, 0x9393ae3dU, 0x26266a4cU,
    0x36365a6cU, 0x3f3f417eU, 0xf7f702f5U, 0xcccc4f83U,
    0x34345c68U, 0xa5a5f451U, 0xe5e534d1U, 0xf1f108f9U,
    0x717193e2U, 0xd8d873abU, 0x31315362U, 0x15153f2aU,
    0x04040c08U, 0xc7c75295U, 0x23236546U, 0xc3c35e9dU,
    0x18182830U, 0x9696a137U, 0x05050f0aU, 0x9a9ab52fU,
    0x0707090eU, 0x12123624U, 0x80809b1bU, 0xe2e23ddfU,
    0xebeb26cdU, 0x2727694eU, 0xb2b2cd7fU, 0x75759feaU,
    0x09091b12U, 0x83839e1dU, 0x2c2c7458U, 0x1a1a2e34U,
    0x1b1b2d36U, 0x6e6eb2dcU, 0x5a5aeeb4U, 0xa0a0fb5bU,
    0x5252f6a4U, 0x3b3b4d76U, 0xd6d661b7U, 0xb3b3ce7dU,
    0x29297b52U, 0xe3e33eddU, 0x2f2f715eU, 0x84849713U,
    0x5353f5a6U, 0xd1d168b9U, 0x00000000U, 0xeded2cc1U,
    0x20206040U, 0xfcfc1fe3U, 0xb1b1c879U, 0x5b5bedb6U,
    0x6a6abed4U, 0xcbcb468dU, 0xbebed967U, 0x39394b72U,
    0x4a4ade94U, 0x4c4cd498U, 0x5858e8b0U, 0xcfcf4a85U,
    0xd0d06bbbU, 0xefef2ac5U, 0xaaaae54fU, 0xfbfb16edU,
    0x4343c586U, 0x4d4dd79aU, 0x33335566U, 0x85859411U,
    0x4545cf8aU, 0xf9f910e9U, 0x02020604U, 0x7f7f81feU,
    0x5050f0a0U, 0x3c3c4478U, 0x9f9fba25U, 0xa8a8e34bU,
    0x5151f3a2U, 0xa3a3fe5dU, 0x4040c080U, 0x8f8f8a05U,
    0x9292ad3fU, 0x9d9dbc21U, 0x38384870U, 0xf5f504f1U,
    0xbcbcdf63U, 0xb6b6c177U, 0xdada75afU, 0x21216342U,
    0x10103020U, 0xffff1ae5U, 0xf3f30efdU, 0xd2d26dbfU,
    0xcdcd4c81U, 0x0c0c1418U, 0x13133526U, 0xecec2fc3U,
    0x5f5fe1beU, 0x9797a235U, 0x4444cc88U, 0x1717392eU,
    0xc4c45793U, 0xa7a7f255U, 0x7e7e82fcU, 0x3d3d477aU,
    0x6464acc8U, 0x5d5de7baU, 0x19192b32U, 0x737395e6U,
    0x6060a0c0U, 0x81819819U, 0x4f4fd19eU, 0xdcdc7fa3U,
    0x22226644U, 0x2a2a7e54U, 0x9090ab3bU, 0x8888830bU,
    0x4646ca8cU, 0xeeee29c7U, 0xb8b8d36bU, 0x14143c28U,
    0xdede79a7U, 0x5e5ee2bcU, 0x0b0b1d16U, 0xdbdb76adU,
    0xe0e03bdbU, 0x32325664U, 0x3a3a4e74U, 0x0a0a1e14U,
    0x4949db92U, 0x06060a0cU, 0x24246c48U, 0x5c5ce4b8U,
    0xc2c25d9fU, 0xd3d36ebdU, 0xacacef43U, 0x6262a6c4U,
    0x9191a839U, 0x9595a431U, 0xe4e437d3U, 0x79798bf2U,
    0xe7e732d5U, 0xc8c8438bU, 0x3737596eU, 0x6d6db7daU,
    0x8d8d8c01U, 0xd5d564b1U, 0x4e4ed29cU, 0xa9a9e049U,
    0x6c6cb4d8U, 0x5656faacU, 0xf4f407f3U, 0xeaea25cfU,
    0x6565afcaU, 0x7a7a8ef4U, 0xaeaee947U, 0x08081810U,
    0xbabad56fU, 0x787888f0U, 0x25256f4aU, 0x2e2e725cU,
    0x1c1c2438U, 0xa6a6f157U, 0xb4b4c773U, 0xc6c65197U,
    0xe8e823cbU, 0xdddd7ca1U, 0x74749ce8U, 0x1f1f213eU,
    0x4b4bdd96U, 0xbdbddc61U, 0x8b8b860dU, 0x8a8a850fU,
    0x707090e0U, 0x3e3e427cU, 0xb5b5c471U, 0x6666aaccU,
    0x4848d890U, 0x03030506U, 0xf6f601f7U, 0x0e0e121cU,
    0x6161a3c2U, 0x35355f6aU, 0x5757f9aeU, 0xb9b9d069U,
    0x86869117U, 0xc1c15899U, 0x1d1d273aU, 0x9e9eb927U,
    0xe1e138d9U, 0xf8f813ebU, 0x9898b32bU, 0x11113322U,
    0x6969bbd2U, 0xd9d970a9U, 0x8e8e8907U, 0x9494a733U,
    0x9b9bb62dU, 0x1e1e223cU, 0x87879215U, 0xe9e920c9U,
    0xcece4987U, 0x5555ffaaU, 0x28287850U, 0xdfdf7aa5U,
    0x8c8c8f03U, 0xa1a1f859U, 0x89898009U, 0x0d0d171aU,
    0xbfbfda65U, 0xe6e631d7U, 0x4242c684U, 0x6868b8d0U,
    0x4141c382U, 0x9999b029U, 0x2d2d775aU, 0x0f0f111eU,
    0xb0b0cb7bU, 0x5454fca8U, 0xbbbbd66dU, 0x16163a2cU,
};

static const u32 Te4[256] = {
    0x63636363U, 0x7c7c7c7cU, 0x77777777U, 0x7b7b7b7bU,
    0xf2f2f2f2U, 0x6b6b6b6bU, 0x6f6f6f6fU, 0xc5c5c5c5U,
    0x30303030U, 0x01010101U, 0x67676767U, 0x2b2b2b2bU,
    0xfefefefeU, 0xd7d7d7d7U, 0xababababU, 0x76767676U,
    0xcacacacaU, 0x82828282U, 0xc9c9c9c9U, 0x7d7d7d7dU,
    0xfafafafaU, 0x59595959U, 0x47474747U, 0xf0f0f0f0U,
    0xadadadadU, 0xd4d4d4d4U, 0xa2a2a2a2U, 0xafafafafU,
    0x9c9c9c9cU, 0xa4a4a4a4U, 0x72727272U, 0xc0c0c0c0U,
    0xb7b7b7b7U, 0xfdfdfdfdU, 0x93939393U, 0x26262626U,
    0x36363636U, 0x3f3f3f3fU, 0xf7f7f7f7U, 0xccccccccU,
    0x34343434U, 0xa5a5a5a5U, 0xe5e5e5e5U, 0xf1f1f1f1U,
    0x71717171U, 0xd8d8d8d8U, 0x31313131U, 0x15151515U,
    0x04040404U, 0xc7c7c7c7U, 0x23232323U, 0xc3c3c3c3U,
    0x18181818U, 0x96969696U, 0x05050505U, 0x9a9a9a9aU,
    0x07070707U, 0x12121212U, 0x80808080U, 0xe2e2e2e2U,
    0xebebebebU, 0x27272727U, 0xb2b2b2b2U, 0x75757575U,
    0x09090909U, 0x83838383U, 0x2c2c2c2cU, 0x1a1a1a1aU,
    0x1b1b1b1bU, 0x6e6e6e6eU, 0x5a5a5a5aU, 0xa0a0a0a0U,
    0x52525252U, 0x3b3b3b3bU, 0xd6d6d6d6U, 0xb3b3b3b3U,
    0x29292929U, 0xe3e3e3e3U, 0x2f2f2f2fU, 0x84848484U,
    0x53535353U, 0xd1d1d1d1U, 0x00000000U, 0xededededU,
    0x20202020U, 0xfcfcfcfcU, 0xb1b1b1b1U, 0x5b5b5b5bU,
    0x6a6a6a6aU, 0xcbcbcbcbU, 0xbebebebeU, 0x39393939U,
    0x4a4a4a4aU, 0x4c4c4c4cU, 0x58585858U, 0xcfcfcfcfU,
    0xd0d0d0d0U, 0xefefefefU, 0xaaaaaaaaU, 0xfbfbfbfbU,
    0x43434343U, 0x4d4d4d4dU, 0x33333333U, 0x85858585U,
    0x45454545U, 0xf9f9f9f9U, 0x02020202U, 0x7f7f7f7fU,
    0x50505050U, 0x3c3c3c3cU, 0x9f9f9f9fU, 0xa8a8a8a8U,
    0x51515151U, 0xa3a3a3a3U, 0x40404040U, 0x8f8f8f8fU,
    0x92929292U, 0x9d9d9d9dU, 0x38383838U, 0xf5f5f5f5U,
    0xbcbcbcbcU, 0xb6b6b6b6U, 0xdadadadaU, 0x21212121U,
    0x10101010U, 0xffffffffU, 0xf3f3f3f3U, 0xd2d2d2d2U,
    0xcdcdcdcdU, 0x0c0c0c0cU, 0x13131313U, 0xececececU,
    0x5f5f5f5fU, 0x97979797U, 0x44444444U, 0x17171717U,
    0xc4c4c4c4U, 0xa7a7a7a7U, 0x7e7e7e7eU, 0x3d3d3d3dU,
    0x64646464U, 0x5d5d5d5dU, 0x19191919U, 0x73737373U,
    0x60606060U, 0x81818181U, 0x4f4f4f4fU, 0xdcdcdcdcU,
    0x22222222U, 0x2a2a2a2aU, 0x90909090U, 0x88888888U,
    0x46464646U, 0xeeeeeeeeU, 0xb8b8b8b8U, 0x14141414U,
    0xdedededeU, 0x5e5e5e5eU, 0x0b0b0b0bU, 0xdbdbdbdbU,
    0xe0e0e0e0U, 0x32323232U, 0x3a3a3a3aU, 0x0a0a0a0aU,
    0x49494949U, 0x06060606U, 0x24242424U, 0x5c5c5c5cU,
    0xc2c2c2c2U, 0xd3d3d3d3U, 0xacacacacU, 0x62626262U,
    0x91919191U, 0x95959595U, 0xe4e4e4e4U, 0x79797979U,
    0xe7e7e7e7U, 0xc8c8c8c8U, 0x37373737U, 0x6d6d6d6dU,
    0x8d8d8d8dU, 0xd5d5d5d5U, 0x4e4e4e4eU, 0xa9a9a9a9U,
    0x6c6c6c6cU, 0x56565656U, 0xf4f4f4f4U, 0xeaeaeaeaU,
    0x65656565U, 0x7a7a7a7aU, 0xaeaeaeaeU, 0x08080808U,
    0xbabababaU, 0x78787878U, 0x25252525U, 0x2e2e2e2eU,
    0x1c1c1c1cU, 0xa6a6a6a6U, 0xb4b4b4b4U, 0xc6c6c6c6U,
    0xe8e8e8e8U, 0xddddddddU, 0x74747474U, 0x1f1f1f1fU,
    0x4b4b4b4bU, 0xbdbdbdbdU, 0x8b8b8b8bU, 0x8a8a8a8aU,
    0x70707070U, 0x3e3e3e3eU, 0xb5b5b5b5U, 0x66666666U,
    0x48484848U, 0x03030303U, 0xf6f6f6f6U, 0x0e0e0e0eU,
    0x61616161U, 0x35353535U, 0x57575757U, 0xb9b9b9b9U,
    0x86868686U, 0xc1c1c1c1U, 0x1d1d1d1dU, 0x9e9e9e9eU,
    0xe1e1e1e1U, 0xf8f8f8f8U, 0x98989898U, 0x11111111U,
    0x69696969U, 0xd9d9d9d9U, 0x8e8e8e8eU, 0x94949494U,
    0x9b9b9b9bU, 0x1e1e1e1eU, 0x87878787U, 0xe9e9e9e9U,
    0xcecececeU, 0x55555555U, 0x28282828U, 0xdfdfdfdfU,
    0x8c8c8c8cU, 0xa1a1a1a1U, 0x89898989U, 0x0d0d0d0dU,
    0xbfbfbfbfU, 0xe6e6e6e6U, 0x42424242U, 0x68686868U,
    0x41414141U, 0x99999999U, 0x2d2d2d2dU, 0x0f0f0f0fU,
    0xb0b0b0b0U, 0x54545454U, 0xbbbbbbbbU, 0x16161616U,
};

static const u32 Td0[256] = {
    0x51f4a750U, 0x7e416553U, 0x1a17a4c3U, 0x3a275e96U,
    0x3bab6bcbU, 0x1f9d45f1U, 0xacfa58abU, 0x4be30393U,
    0x2030fa55U, 0xad766df6U, 0x88cc7691U, 0xf5024c25U,
    0x4fe5d7fcU, 0xc52acbd7U, 0x26354480U, 0xb562a38fU,
    0xdeb15a49U, 0x25ba1b67U, 0x45ea0e98U, 0x5dfec0e1U,
    0xc32f7502U, 0x814cf012U, 0x8d4697a3U, 0x6bd3f9c6U,
    0x038f5fe7U, 0x15929c95U, 0xbf6d7aebU, 0x955259daU,
    0xd4be832dU, 0x587421d3U, 0x49e06929U, 0x8ec9c844U,
    0x75c2896aU, 0xf48e7978U, 0x99583e6bU, 0x27b971ddU,
    0xbee14fb6U, 0xf088ad17U, 0xc920ac66U, 0x7dce3ab4U,
    0x63df4a18U, 0xe51a3182U, 0x97513360U, 0x62537f45U,
    0xb16477e0U, 0xbb6bae84U, 0xfe81a01cU, 0xf9082b94U,
    0x70486858U, 0x8f45fd19U, 0x94de6c87U, 0x527bf8b7U,
    0xab73d323U, 0x724b02e2U, 0xe31f8f57U, 0x6655ab2aU,
    0xb2eb2807U, 0x2fb5c203U, 0x86c57b9aU, 0xd33708a5U,
    0x302887f2U, 0x23bfa5b2U, 0x02036abaU, 0xed16825cU,
    0x8acf1c2bU, 0xa779b492U, 0xf307f2f0U, 0x4e69e2a1U,
    0x65daf4cdU, 0x0605bed5U, 0xd134621fU, 0xc4a6fe8aU,
    0x342e539dU, 0xa2f355a0U, 0x058ae132U, 0xa4f6eb75U,
    0x0b83ec39U, 0x4060efaaU, 0x5e719f06U, 0xbd6e1051U,
    0x3e218af9U, 0x96dd063dU, 0xdd3e05aeU, 0x4de6bd46U,
    0x91548db5U, 0x71c45d05U, 0x0406d46fU, 0x605015ffU,
    0x1998fb24U, 0xd6bde997U, 0x894043ccU, 0x67d99e77U,
    0xb0e842bdU, 0x07898b88U, 0xe7195b38U, 0x79c8eedbU,
    0xa17c0a47U, 0x7c420fe9U, 0xf8841ec9U, 0x00000000U,
    0x09808683U, 0x322bed48U, 0x1e1170acU, 0x6c5a724eU,
    0xfd0efffbU, 0x0f853856U, 0x3daed51eU, 0x362d3927U,
    0x0a0fd964U, 0x685ca621U, 0x9b5b54d1U, 0x24362e3aU,
    0x0c0a67b1U, 0x9357e70fU, 0xb4ee96d2U, 0x1b9b919eU,
    0x80c0c54fU, 0x61dc20a2U, 0x5a774b69U, 0x1c121a16U,
    0xe293ba0aU, 0xc0a02ae5U, 0x3c22e043U, 0x121b171dU,
    0x0e090d0bU, 0xf28bc7adU, 0x2db6a8b9U, 0x141ea9c8U,
    0x57f11985U, 0xaf75074cU, 0xee99ddbbU, 0xa37f60fdU,
    0xf701269fU, 0x5c72f5bcU, 0x44663bc5U, 0x5bfb7e34U,
    0x8b432976U, 0xcb23c6dcU, 0xb6edfc68U, 0xb8e4f163U,
    0xd731dccaU, 0x42638510U, 0x13972240U, 0x84c61120U,
    0x854a247dU, 0xd2bb3df8U, 0xaef93211U, 0xc729a16dU,
    0x1d9e2f4bU, 0xdcb230f3U, 0x0d8652ecU, 0x77c1e3d0U,
    0x2bb3166cU, 0xa970b999U, 0x119448faU, 0x47e96422U,
    0xa8fc8cc4U, 0xa0f03f1aU, 0x567d2cd8U, 0x223390efU,
    0x87494ec7U, 0xd938d1c1U, 0x8ccaa2feU, 0x98d40b36U,
    0xa6f581cfU, 0xa57ade28U, 0xdab78e26U, 0x3fadbfa4U,
    0x2c3a9de4U, 0x5078920dU, 0x6a5fcc9bU, 0x547e4662U,
    0xf68d13c2U, 0x90d8b8e8U, 0x2e39f75eU, 0x82c3aff5U,
    0x9f5d80beU, 0x69d0937cU, 0x6fd52da9U, 0xcf2512b3U,
    0xc8ac993bU, 0x10187da7U, 0xe89c636eU, 0xdb3bbb7bU,
    0xcd267809U, 0x6e5918f4U, 0xec9ab701U, 0x834f9aa8U,
    0xe6956e65U, 0xaaffe67eU, 0x21bccf08U, 0xef15e8e6U,
    0xbae79bd9U, 0x4a6f36ceU, 0xea9f09d4U, 0x29b07cd6U,
    0x31a4b2afU, 0x2a3f2331U, 0xc6a59430U, 0x35a266c0U,
    0x744ebc37U, 0xfc82caa6U, 0xe090d0b0U, 0x33a7d815U,
    0xf104984aU, 0x41ecdaf7U, 0x7fcd500eU, 0x1791f62fU,
    0x764dd68dU, 0x43efb04dU, 0xccaa4d54U, 0xe49604dfU,
    0x9ed1b5e3U, 0x4c6a881bU, 0xc12c1fb8U, 0x4665517fU,
    0x9d5eea04U, 0x018c355dU, 0xfa877473U, 0xfb0b412eU,
    0xb3671d5aU, 0x92dbd252U, 0xe9105633U, 0x6dd64713U,
    0x9ad7618cU, 0x37a10c7aU, 0x59f8148eU, 0xeb133c89U,
    0xcea927eeU, 0xb761c935U, 0xe11ce5edU, 0x7a47b13cU,
    0x9cd2df59U, 0x55f2733fU, 0x1814ce79U, 0x73c737bfU,
    0x53f7cdeaU, 0x5ffdaa5bU, 0xdf3d6f14U, 0x7844db86U,
    0xcaaff381U, 0xb968c43eU, 0x3824342cU, 0xc2a3405fU,
    0x161dc372U, 0xbce2250cU, 0x283c498bU, 0xff0d9541U,
    0x39a80171U, 0x080cb3deU, 0xd8b4e49cU, 0x6456c190U,
    0x7bcb8461U, 0xd532b670U, 0x486c5c74U, 0xd0b85742U,
};
static const u32 Td1[256] = {
    0x5051f4a7U, 0x537e4165U, 0xc31a17a4U, 0x963a275eU,
    0xcb3bab6bU, 0xf11f9d45U, 0xabacfa58U, 0x934be303U,
    0x552030faU, 0xf6ad766dU, 0x9188cc76U, 0x25f5024cU,
    0xfc4fe5d7U, 0xd7c52acbU, 0x80263544U, 0x8fb562a3U,
    0x49deb15aU, 0x6725ba1bU, 0x9845ea0eU, 0xe15dfec0U,
    0x02c32f75U, 0x12814cf0U, 0xa38d4697U, 0xc66bd3f9U,
    0xe7038f5fU, 0x9515929cU, 0xebbf6d7aU, 0xda955259U,
    0x2dd4be83U, 0xd3587421U, 0x2949e069U, 0x448ec9c8U,
    0x6a75c289U, 0x78f48e79U, 0x6b99583eU, 0xdd27b971U,
    0xb6bee14fU, 0x17f088adU, 0x66c920acU, 0xb47dce3aU,
    0x1863df4aU, 0x82e51a31U, 0x60975133U, 0x4562537fU,
    0xe0b16477U, 0x84bb6baeU, 0x1cfe81a0U, 0x94f9082bU,
    0x58704868U, 0x198f45fdU, 0x8794de6cU, 0xb7527bf8U,
    0x23ab73d3U, 0xe2724b02U, 0x57e31f8fU, 0x2a6655abU,
    0x07b2eb28U, 0x032fb5c2U, 0x9a86c57bU, 0xa5d33708U,
    0xf2302887U, 0xb223bfa5U, 0xba02036aU, 0x5ced1682U,
    0x2b8acf1cU, 0x92a779b4U, 0xf0f307f2U, 0xa14e69e2U,
    0xcd65daf4U, 0xd50605beU, 0x1fd13462U, 0x8ac4a6feU,
    0x9d342e53U, 0xa0a2f355U, 0x32058ae1U, 0x75a4f6ebU,
    0x390b83ecU, 0xaa4060efU, 0x065e719fU, 0x51bd6e10U,
    0xf93e218aU, 0x3d96dd06U, 0xaedd3e05U, 0x464de6bdU,
    0xb591548dU, 0x0571c45dU, 0x6f0406d4U, 0xff605015U,
    0x241998fbU, 0x97d6bde9U, 0xcc894043U, 0x7767d99eU,
    0xbdb0e842U, 0x8807898bU, 0x38e7195bU, 0xdb79c8eeU,
    0x47a17c0aU, 0xe97c420fU, 0xc9f8841eU, 0x00000000U,
    0x83098086U, 0x48322bedU, 0xac1e1170U, 0x4e6c5a72U,
    0xfbfd0effU, 0x560f8538U, 0x1e3daed5U, 0x27362d39U,
    0x640a0fd9U, 0x21685ca6U, 0xd19b5b54U, 0x3a24362eU,
    0xb10c0a67U, 0x0f9357e7U, 0xd2b4ee96U, 0x9e1b9b91U,
    0x4f80c0c5U, 0xa261dc20U, 0x695a774bU, 0x161c121aU,
    0x0ae293baU, 0xe5c0a02aU, 0x433c22e0U, 0x1d121b17U,
    0x0b0e090dU, 0xadf28bc7U, 0xb92db6a8U, 0xc8141ea9U,
    0x8557f119U, 0x4caf7507U, 0xbbee99ddU, 0xfda37f60U,
    0x9ff70126U, 0xbc5c72f5U, 0xc544663bU, 0x345bfb7eU,
    0x768b4329U, 0xdccb23c6U, 0x68b6edfcU, 0x63b8e4f1U,
    0xcad731dcU, 0x10426385U, 0x40139722U, 0x2084c611U,
    0x7d854a24U, 0xf8d2bb3dU, 0x11aef932U, 0x6dc729a1U,
    0x4b1d9e2fU, 0xf3dcb230U, 0xec0d8652U, 0xd077c1e3U,
    0x6c2bb316U, 0x99a970b9U, 0xfa119448U, 0x2247e964U,
    0xc4a8fc8cU, 0x1aa0f03fU, 0xd8567d2cU, 0xef223390U,
    0xc787494eU, 0xc1d938d1U, 0xfe8ccaa2U, 0x3698d40bU,
    0xcfa6f581U, 0x28a57adeU, 0x26dab78eU, 0xa43fadbfU,
    0xe42c3a9dU, 0x0d507892U, 0x9b6a5fccU, 0x62547e46U,
    0xc2f68d13U, 0xe890d8b8U, 0x5e2e39f7U, 0xf582c3afU,
    0xbe9f5d80U, 0x7c69d093U, 0xa96fd52dU, 0xb3cf2512U,
    0x3bc8ac99U, 0xa710187dU, 0x6ee89c63U, 0x7bdb3bbbU,
    0x09cd2678U, 0xf46e5918U, 0x01ec9ab7U, 0xa8834f9aU,
    0x65e6956eU, 0x7eaaffe6U, 0x0821bccfU, 0xe6ef15e8U,
    0xd9bae79bU, 0xce4a6f36U, 0xd4ea9f09U, 0xd629b07cU,
    0xaf31a4b2U, 0x312a3f23U, 0x30c6a594U, 0xc035a266U,
    0x37744ebcU, 0xa6fc82caU, 0xb0e090d0U, 0x1533a7d8U,
    0x4af10498U, 0xf741ecdaU, 0x0e7fcd50U, 0x2f1791f6U,
    0x8d764dd6U, 0x4d43efb0U, 0x54ccaa4dU, 0xdfe49604U,
    0xe39ed1b5U, 0x1b4c6a88U, 0xb8c12c1fU, 0x7f466551U,
    0x049d5eeaU, 0x5d018c35U, 0x73fa8774U, 0x2efb0b41U,
    0x5ab3671dU, 0x5292dbd2U, 0x33e91056U, 0x136dd647U,
    0x8c9ad761U, 0x7a37a10cU, 0x8e59f814U, 0x89eb133cU,
    0xeecea927U, 0x35b761c9U, 0xede11ce5U, 0x3c7a47b1U,
    0x599cd2dfU, 0x3f55f273U, 0x791814ceU, 0xbf73c737U,
    0xea53f7cdU, 0x5b5ffdaaU, 0x14df3d6fU, 0x867844dbU,
    0x81caaff3U, 0x3eb968c4U, 0x2c382434U, 0x5fc2a340U,
    0x72161dc3U, 0x0cbce225U, 0x8b283c49U, 0x41ff0d95U,
    0x7139a801U, 0xde080cb3U, 0x9cd8b4e4U, 0x906456c1U,
    0x617bcb84U, 0x70d532b6U, 0x74486c5cU, 0x42d0b857U,
};
static const u32 Td2[256] = {
    0xa75051f4U, 0x65537e41U, 0xa4c31a17U, 0x5e963a27U,
    0x6bcb3babU, 0x45f11f9dU, 0x58abacfaU, 0x03934be3U,
    0xfa552030U, 0x6df6ad76U, 0x769188ccU, 0x4c25f502U,
    0xd7fc4fe5U, 0xcbd7c52aU, 0x44802635U, 0xa38fb562U,
    0x5a49deb1U, 0x1b6725baU, 0x0e9845eaU, 0xc0e15dfeU,
    0x7502c32fU, 0xf012814cU, 0x97a38d46U, 0xf9c66bd3U,
    0x5fe7038fU, 0x9c951592U, 0x7aebbf6dU, 0x59da9552U,
    0x832dd4beU, 0x21d35874U, 0x692949e0U, 0xc8448ec9U,
    0x896a75c2U, 0x7978f48eU, 0x3e6b9958U, 0x71dd27b9U,
    0x4fb6bee1U, 0xad17f088U, 0xac66c920U, 0x3ab47dceU,
    0x4a1863dfU, 0x3182e51aU, 0x33609751U, 0x7f456253U,
    0x77e0b164U, 0xae84bb6bU, 0xa01cfe81U, 0x2b94f908U,
    0x68587048U, 0xfd198f45U, 0x6c8794deU, 0xf8b7527bU,
    0xd323ab73U, 0x02e2724bU, 0x8f57e31fU, 0xab2a6655U,
    0x2807b2ebU, 0xc2032fb5U, 0x7b9a86c5U, 0x08a5d337U,
    0x87f23028U, 0xa5b223bfU, 0x6aba0203U, 0x825ced16U,
    0x1c2b8acfU, 0xb492a779U, 0xf2f0f307U, 0xe2a14e69U,
    0xf4cd65daU, 0xbed50605U, 0x621fd134U, 0xfe8ac4a6U,
    0x539d342eU, 0x55a0a2f3U, 0xe132058aU, 0xeb75a4f6U,
    0xec390b83U, 0xefaa4060U, 0x9f065e71U, 0x1051bd6eU,
    0x8af93e21U, 0x063d96ddU, 0x05aedd3eU, 0xbd464de6U,
    0x8db59154U, 0x5d0571c4U, 0xd46f0406U, 0x15ff6050U,
    0xfb241998U, 0xe997d6bdU, 0x43cc8940U, 0x9e7767d9U,
    0x42bdb0e8U, 0x8b880789U, 0x5b38e719U, 0xeedb79c8U,
    0x0a47a17cU, 0x0fe97c42U, 0x1ec9f884U, 0x00000000U,
    0x86830980U, 0xed48322bU, 0x70ac1e11U, 0x724e6c5aU,
    0xfffbfd0eU, 0x38560f85U, 0xd51e3daeU, 0x3927362dU,
    0xd9640a0fU, 0xa621685cU, 0x54d19b5bU, 0x2e3a2436U,
    0x67b10c0aU, 0xe70f9357U, 0x96d2b4eeU, 0x919e1b9bU,
    0xc54f80c0U, 0x20a261dcU, 0x4b695a77U, 0x1a161c12U,
    0xba0ae293U, 0x2ae5c0a0U, 0xe0433c22U, 0x171d121bU,
    0x0d0b0e09U, 0xc7adf28bU, 0xa8b92db6U, 0xa9c8141eU,
    0x198557f1U, 0x074caf75U, 0xddbbee99U, 0x60fda37fU,
    0x269ff701U, 0xf5bc5c72U, 0x3bc54466U, 0x7e345bfbU,
    0x29768b43U, 0xc6dccb23U, 0xfc68b6edU, 0xf163b8e4U,
    0xdccad731U, 0x85104263U, 0x22401397U, 0x112084c6U,
    0x247d854aU, 0x3df8d2bbU, 0x3211aef9U, 0xa16dc729U,
    0x2f4b1d9eU, 0x30f3dcb2U, 0x52ec0d86U, 0xe3d077c1U,
    0x166c2bb3U, 0xb999a970U, 0x48fa1194U, 0x642247e9U,
    0x8cc4a8fcU, 0x3f1aa0f0U, 0x2cd8567dU, 0x90ef2233U,
    0x4ec78749U, 0xd1c1d938U, 0xa2fe8ccaU, 0x0b3698d4U,
    0x81cfa6f5U, 0xde28a57aU, 0x8e26dab7U, 0xbfa43fadU,
    0x9de42c3aU, 0x920d5078U, 0xcc9b6a5fU, 0x4662547eU,
    0x13c2f68dU, 0xb8e890d8U, 0xf75e2e39U, 0xaff582c3U,
    0x80be9f5dU, 0x937c69d0U, 0x2da96fd5U, 0x12b3cf25U,
    0x993bc8acU, 0x7da71018U, 0x636ee89cU, 0xbb7bdb3bU,
    0x7809cd26U, 0x18f46e59U, 0xb701ec9aU, 0x9aa8834fU,
    0x6e65e695U, 0xe67eaaffU, 0xcf0821bcU, 0xe8e6ef15U,
    0x9bd9bae7U, 0x36ce4a6fU, 0x09d4ea9fU, 0x7cd629b0U,
    0xb2af31a4U, 0x23312a3fU, 0x9430c6a5U, 0x66c035a2U,
    0xbc37744eU, 0xcaa6fc82U, 0xd0b0e090U, 0xd81533a7U,
    0x984af104U, 0xdaf741ecU, 0x500e7fcdU, 0xf62f1791U,
    0xd68d764dU, 0xb04d43efU, 0x4d54ccaaU, 0x04dfe496U,
    0xb5e39ed1U, 0x881b4c6aU, 0x1fb8c12cU, 0x517f4665U,
    0xea049d5eU, 0x355d018cU, 0x7473fa87U, 0x412efb0bU,
    0x1d5ab367U, 0xd25292dbU, 0x5633e910U, 0x47136dd6U,
    0x618c9ad7U, 0x0c7a37a1U, 0x148e59f8U, 0x3c89eb13U,
    0x27eecea9U, 0xc935b761U, 0xe5ede11cU, 0xb13c7a47U,
    0xdf599cd2U, 0x733f55f2U, 0xce791814U, 0x37bf73c7U,
    0xcdea53f7U, 0xaa5b5ffdU, 0x6f14df3dU, 0xdb867844U,
    0xf381caafU, 0xc43eb968U, 0x342c3824U, 0x405fc2a3U,
    0xc372161dU, 0x250cbce2U, 0x498b283cU, 0x9541ff0dU,
    0x017139a8U, 0xb3de080cU, 0xe49cd8b4U, 0xc1906456U,
    0x84617bcbU, 0xb670d532U, 0x5c74486cU, 0x5742d0b8U,
};
static const u32 Td3[256] = {
    0xf4a75051U, 0x4165537eU, 0x17a4c31aU, 0x275e963aU,
    0xab6bcb3bU, 0x9d45f11fU, 0xfa58abacU, 0xe303934bU,
    0x30fa5520U, 0x766df6adU, 0xcc769188U, 0x024c25f5U,
    0xe5d7fc4fU, 0x2acbd7c5U, 0x35448026U, 0x62a38fb5U,
    0xb15a49deU, 0xba1b6725U, 0xea0e9845U, 0xfec0e15dU,
    0x2f7502c3U, 0x4cf01281U, 0x4697a38dU, 0xd3f9c66bU,
    0x8f5fe703U, 0x929c9515U, 0x6d7aebbfU, 0x5259da95U,
    0xbe832dd4U, 0x7421d358U, 0xe0692949U, 0xc9c8448eU,
    0xc2896a75U, 0x8e7978f4U, 0x583e6b99U, 0xb971dd27U,
    0xe14fb6beU, 0x88ad17f0U, 0x20ac66c9U, 0xce3ab47dU,
    0xdf4a1863U, 0x1a3182e5U, 0x51336097U, 0x537f4562U,
    0x6477e0b1U, 0x6bae84bbU, 0x81a01cfeU, 0x082b94f9U,
    0x48685870U, 0x45fd198fU, 0xde6c8794U, 0x7bf8b752U,
    0x73d323abU, 0x4b02e272U, 0x1f8f57e3U, 0x55ab2a66U,
    0xeb2807b2U, 0xb5c2032fU, 0xc57b9a86U, 0x3708a5d3U,
    0x2887f230U, 0xbfa5b223U, 0x036aba02U, 0x16825cedU,
    0xcf1c2b8aU, 0x79b492a7U, 0x07f2f0f3U, 0x69e2a14eU,
    0xdaf4cd65U, 0x05bed506U, 0x34621fd1U, 0xa6fe8ac4U,
    0x2e539d34U, 0xf355a0a2U, 0x8ae13205U, 0xf6eb75a4U,
    0x83ec390bU, 0x60efaa40U, 0x719f065eU, 0x6e1051bdU,
    0x218af93eU, 0xdd063d96U, 0x3e05aeddU, 0xe6bd464dU,
    0x548db591U, 0xc45d0571U, 0x06d46f04U, 0x5015ff60U,
    0x98fb2419U, 0xbde997d6U, 0x4043cc89U, 0xd99e7767U,
    0xe842bdb0U, 0x898b8807U, 0x195b38e7U, 0xc8eedb79U,
    0x7c0a47a1U, 0x420fe97cU, 0x841ec9f8U, 0x00000000U,
    0x80868309U, 0x2bed4832U, 0x1170ac1eU, 0x5a724e6cU,
    0x0efffbfdU, 0x8538560fU, 0xaed51e3dU, 0x2d392736U,
    0x0fd9640aU, 0x5ca62168U, 0x5b54d19bU, 0x362e3a24U,
    0x0a67b10cU, 0x57e70f93U, 0xee96d2b4U, 0x9b919e1bU,
    0xc0c54f80U, 0xdc20a261U, 0x774b695aU, 0x121a161cU,
    0x93ba0ae2U, 0xa02ae5c0U, 0x22e0433cU, 0x1b171d12U,
    0x090d0b0eU, 0x8bc7adf2U, 0xb6a8b92dU, 0x1ea9c814U,
    0xf1198557U, 0x75074cafU, 0x99ddbbeeU, 0x7f60fda3U,
    0x01269ff7U, 0x72f5bc5cU, 0x663bc544U, 0xfb7e345bU,
    0x4329768bU, 0x23c6dccbU, 0xedfc68b6U, 0xe4f163b8U,
    0x31dccad7U, 0x63851042U, 0x97224013U, 0xc6112084U,
    0x4a247d85U, 0xbb3df8d2U, 0xf93211aeU, 0x29a16dc7U,
    0x9e2f4b1dU, 0xb230f3dcU, 0x8652ec0dU, 0xc1e3d077U,
    0xb3166c2bU, 0x70b999a9U, 0x9448fa11U, 0xe9642247U,
    0xfc8cc4a8U, 0xf03f1aa0U, 0x7d2cd856U, 0x3390ef22U,
    0x494ec787U, 0x38d1c1d9U, 0xcaa2fe8cU, 0xd40b3698U,
    0xf581cfa6U, 0x7ade28a5U, 0xb78e26daU, 0xadbfa43fU,
    0x3a9de42cU, 0x78920d50U, 0x5fcc9b6aU, 0x7e466254U,
    0x8d13c2f6U, 0xd8b8e890U, 0x39f75e2eU, 0xc3aff582U,
    0x5d80be9fU, 0xd0937c69U, 0xd52da96fU, 0x2512b3cfU,
    0xac993bc8U, 0x187da710U, 0x9c636ee8U, 0x3bbb7bdbU,
    0x267809cdU, 0x5918f46eU, 0x9ab701ecU, 0x4f9aa883U,
    0x956e65e6U, 0xffe67eaaU, 0xbccf0821U, 0x15e8e6efU,
    0xe79bd9baU, 0x6f36ce4aU, 0x9f09d4eaU, 0xb07cd629U,
    0xa4b2af31U, 0x3f23312aU, 0xa59430c6U, 0xa266c035U,
    0x4ebc3774U, 0x82caa6fcU, 0x90d0b0e0U, 0xa7d81533U,
    0x04984af1U, 0xecdaf741U, 0xcd500e7fU, 0x91f62f17U,
    0x4dd68d76U, 0xefb04d43U, 0xaa4d54ccU, 0x9604dfe4U,
    0xd1b5e39eU, 0x6a881b4cU, 0x2c1fb8c1U, 0x65517f46U,
    0x5eea049dU, 0x8c355d01U, 0x877473faU, 0x0b412efbU,
    0x671d5ab3U, 0xdbd25292U, 0x105633e9U, 0xd647136dU,
    0xd7618c9aU, 0xa10c7a37U, 0xf8148e59U, 0x133c89ebU,
    0xa927eeceU, 0x61c935b7U, 0x1ce5ede1U, 0x47b13c7aU,
    0xd2df599cU, 0xf2733f55U, 0x14ce7918U, 0xc737bf73U,
    0xf7cdea53U, 0xfdaa5b5fU, 0x3d6f14dfU, 0x44db8678U,
    0xaff381caU, 0x68c43eb9U, 0x24342c38U, 0xa3405fc2U,
    0x1dc37216U, 0xe2250cbcU, 0x3c498b28U, 0x0d9541ffU,
    0xa8017139U, 0x0cb3de08U, 0xb4e49cd8U, 0x56c19064U,
    0xcb84617bU, 0x32b670d5U, 0x6c5c7448U, 0xb85742d0U,
};

static const u32 Td4[256] = {
    0x52525252U, 0x09090909U, 0x6a6a6a6aU, 0xd5d5d5d5U,
    0x30303030U, 0x36363636U, 0xa5a5a5a5U, 0x38383838U,
    0xbfbfbfbfU, 0x40404040U, 0xa3a3a3a3U, 0x9e9e9e9eU,
    0x81818181U, 0xf3f3f3f3U, 0xd7d7d7d7U, 0xfbfbfbfbU,
    0x7c7c7c7cU, 0xe3e3e3e3U, 0x39393939U, 0x82828282U,
    0x9b9b9b9bU, 0x2f2f2f2fU, 0xffffffffU, 0x87878787U,
    0x34343434U, 0x8e8e8e8eU, 0x43434343U, 0x44444444U,
    0xc4c4c4c4U, 0xdedededeU, 0xe9e9e9e9U, 0xcbcbcbcbU,
    0x54545454U, 0x7b7b7b7bU, 0x94949494U, 0x32323232U,
    0xa6a6a6a6U, 0xc2c2c2c2U, 0x23232323U, 0x3d3d3d3dU,
    0xeeeeeeeeU, 0x4c4c4c4cU, 0x95959595U, 0x0b0b0b0bU,
    0x42424242U, 0xfafafafaU, 0xc3c3c3c3U, 0x4e4e4e4eU,
    0x08080808U, 0x2e2e2e2eU, 0xa1a1a1a1U, 0x66666666U,
    0x28282828U, 0xd9d9d9d9U, 0x24242424U, 0xb2b2b2b2U,
    0x76767676U, 0x5b5b5b5bU, 0xa2a2a2a2U, 0x49494949U,
    0x6d6d6d6dU, 0x8b8b8b8bU, 0xd1d1d1d1U, 0x25252525U,
    0x72727272U, 0xf8f8f8f8U, 0xf6f6f6f6U, 0x64646464U,
    0x86868686U, 0x68686868U, 0x98989898U, 0x16161616U,
    0xd4d4d4d4U, 0xa4a4a4a4U, 0x5c5c5c5cU, 0xccccccccU,
    0x5d5d5d5dU, 0x65656565U, 0xb6b6b6b6U, 0x92929292U,
    0x6c6c6c6cU, 0x70707070U, 0x48484848U, 0x50505050U,
    0xfdfdfdfdU, 0xededededU, 0xb9b9b9b9U, 0xdadadadaU,
    0x5e5e5e5eU, 0x15151515U, 0x46464646U, 0x57575757U,
    0xa7a7a7a7U, 0x8d8d8d8dU, 0x9d9d9d9dU, 0x84848484U,
    0x90909090U, 0xd8d8d8d8U, 0xababababU, 0x00000000U,
    0x8c8c8c8cU, 0xbcbcbcbcU, 0xd3d3d3d3U, 0x0a0a0a0aU,
    0xf7f7f7f7U, 0xe4e4e4e4U, 0x58585858U, 0x05050505U,
    0xb8b8b8b8U, 0xb3b3b3b3U, 0x45454545U, 0x06060606U,
    0xd0d0d0d0U, 0x2c2c2c2cU, 0x1e1e1e1eU, 0x8f8f8f8fU,
    0xcacacacaU, 0x3f3f3f3fU, 0x0f0f0f0fU, 0x02020202U,
    0xc1c1c1c1U, 0xafafafafU, 0xbdbdbdbdU, 0x03030303U,
    0x01010101U, 0x13131313U, 0x8a8a8a8aU, 0x6b6b6b6bU,
    0x3a3a3a3aU, 0x91919191U, 0x11111111U, 0x41414141U,
    0x4f4f4f4fU, 0x67676767U, 0xdcdcdcdcU, 0xeaeaeaeaU,
    0x97979797U, 0xf2f2f2f2U, 0xcfcfcfcfU, 0xcecececeU,
    0xf0f0f0f0U, 0xb4b4b4b4U, 0xe6e6e6e6U, 0x73737373U,
    0x96969696U, 0xacacacacU, 0x74747474U, 0x22222222U,
    0xe7e7e7e7U, 0xadadadadU, 0x35353535U, 0x85858585U,
    0xe2e2e2e2U, 0xf9f9f9f9U, 0x37373737U, 0xe8e8e8e8U,
    0x1c1c1c1cU, 0x75757575U, 0xdfdfdfdfU, 0x6e6e6e6eU,
    0x47474747U, 0xf1f1f1f1U, 0x1a1a1a1aU, 0x71717171U,
    0x1d1d1d1dU, 0x29292929U, 0xc5c5c5c5U, 0x89898989U,
    0x6f6f6f6fU, 0xb7b7b7b7U, 0x62626262U, 0x0e0e0e0eU,
    0xaaaaaaaaU, 0x18181818U, 0xbebebebeU, 0x1b1b1b1bU,
    0xfcfcfcfcU, 0x56565656U, 0x3e3e3e3eU, 0x4b4b4b4bU,
    0xc6c6c6c6U, 0xd2d2d2d2U, 0x79797979U, 0x20202020U,
    0x9a9a9a9aU, 0xdbdbdbdbU, 0xc0c0c0c0U, 0xfefefefeU,
    0x78787878U, 0xcdcdcdcdU, 0x5a5a5a5aU, 0xf4f4f4f4U,
    0x1f1f1f1fU, 0xddddddddU, 0xa8a8a8a8U, 0x33333333U,
    0x88888888U, 0x07070707U, 0xc7c7c7c7U, 0x31313131U,
    0xb1b1b1b1U, 0x12121212U, 0x10101010U, 0x59595959U,
    0x27272727U, 0x80808080U, 0xececececU, 0x5f5f5f5fU,
    0x60606060U, 0x51515151U, 0x7f7f7f7fU, 0xa9a9a9a9U,
    0x19191919U, 0xb5b5b5b5U, 0x4a4a4a4aU, 0x0d0d0d0dU,
    0x2d2d2d2dU, 0xe5e5e5e5U, 0x7a7a7a7aU, 0x9f9f9f9fU,
    0x93939393U, 0xc9c9c9c9U, 0x9c9c9c9cU, 0xefefefefU,
    0xa0a0a0a0U, 0xe0e0e0e0U, 0x3b3b3b3bU, 0x4d4d4d4dU,
    0xaeaeaeaeU, 0x2a2a2a2aU, 0xf5f5f5f5U, 0xb0b0b0b0U,
    0xc8c8c8c8U, 0xebebebebU, 0xbbbbbbbbU, 0x3c3c3c3cU,
    0x83838383U, 0x53535353U, 0x99999999U, 0x61616161U,
    0x17171717U, 0x2b2b2b2bU, 0x04040404U, 0x7e7e7e7eU,
    0xbabababaU, 0x77777777U, 0xd6d6d6d6U, 0x26262626U,
    0xe1e1e1e1U, 0x69696969U, 0x14141414U, 0x63636363U,
    0x55555555U, 0x21212121U, 0x0c0c0c0cU, 0x7d7d7d7dU,
};

static const u32 rcon[] = {
	0x01000000, 0x02000000, 0x04000000, 0x08000000,
	0x10000000, 0x20000000, 0x40000000, 0x80000000,
	0x1B000000, 0x36000000, /* for 128-bit blocks, Rijndael never uses more than 10 rcon values */
};

/*
 * Encrypt a single block
 * in and out can overlap
 */
void evp_aes_encrypt(const unsigned char *in, unsigned char *out, const AES_KEY *key) 
{

	const u32 *rk;
	u32 s0, s1, s2, s3, t0, t1, t2, t3;
#ifndef FULL_UNROLL
	int r;
#endif /* ?FULL_UNROLL */

	assert(in && out && key);
	rk = key->rd_key;

	/*
	 * map byte array block to cipher state
	 * and add initial round key:
	 */
	s0 = GETU32(in     ) ^ rk[0];
	s1 = GETU32(in +  4) ^ rk[1];
	s2 = GETU32(in +  8) ^ rk[2];
	s3 = GETU32(in + 12) ^ rk[3];
#ifdef FULL_UNROLL
	/* round 1: */
   	t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >>  8) & 0xff] ^ Te3[s3 & 0xff] ^ rk[ 4];
   	t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >>  8) & 0xff] ^ Te3[s0 & 0xff] ^ rk[ 5];
   	t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >>  8) & 0xff] ^ Te3[s1 & 0xff] ^ rk[ 6];
   	t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >>  8) & 0xff] ^ Te3[s2 & 0xff] ^ rk[ 7];
   	/* round 2: */
   	s0 = Te0[t0 >> 24] ^ Te1[(t1 >> 16) & 0xff] ^ Te2[(t2 >>  8) & 0xff] ^ Te3[t3 & 0xff] ^ rk[ 8];
   	s1 = Te0[t1 >> 24] ^ Te1[(t2 >> 16) & 0xff] ^ Te2[(t3 >>  8) & 0xff] ^ Te3[t0 & 0xff] ^ rk[ 9];
   	s2 = Te0[t2 >> 24] ^ Te1[(t3 >> 16) & 0xff] ^ Te2[(t0 >>  8) & 0xff] ^ Te3[t1 & 0xff] ^ rk[10];
   	s3 = Te0[t3 >> 24] ^ Te1[(t0 >> 16) & 0xff] ^ Te2[(t1 >>  8) & 0xff] ^ Te3[t2 & 0xff] ^ rk[11];
	/* round 3: */
   	t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >>  8) & 0xff] ^ Te3[s3 & 0xff] ^ rk[12];
   	t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >>  8) & 0xff] ^ Te3[s0 & 0xff] ^ rk[13];
   	t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >>  8) & 0xff] ^ Te3[s1 & 0xff] ^ rk[14];
   	t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >>  8) & 0xff] ^ Te3[s2 & 0xff] ^ rk[15];
   	/* round 4: */
   	s0 = Te0[t0 >> 24] ^ Te1[(t1 >> 16) & 0xff] ^ Te2[(t2 >>  8) & 0xff] ^ Te3[t3 & 0xff] ^ rk[16];
   	s1 = Te0[t1 >> 24] ^ Te1[(t2 >> 16) & 0xff] ^ Te2[(t3 >>  8) & 0xff] ^ Te3[t0 & 0xff] ^ rk[17];
   	s2 = Te0[t2 >> 24] ^ Te1[(t3 >> 16) & 0xff] ^ Te2[(t0 >>  8) & 0xff] ^ Te3[t1 & 0xff] ^ rk[18];
   	s3 = Te0[t3 >> 24] ^ Te1[(t0 >> 16) & 0xff] ^ Te2[(t1 >>  8) & 0xff] ^ Te3[t2 & 0xff] ^ rk[19];
	/* round 5: */
   	t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >>  8) & 0xff] ^ Te3[s3 & 0xff] ^ rk[20];
   	t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >>  8) & 0xff] ^ Te3[s0 & 0xff] ^ rk[21];
   	t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >>  8) & 0xff] ^ Te3[s1 & 0xff] ^ rk[22];
   	t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >>  8) & 0xff] ^ Te3[s2 & 0xff] ^ rk[23];
   	/* round 6: */
   	s0 = Te0[t0 >> 24] ^ Te1[(t1 >> 16) & 0xff] ^ Te2[(t2 >>  8) & 0xff] ^ Te3[t3 & 0xff] ^ rk[24];
   	s1 = Te0[t1 >> 24] ^ Te1[(t2 >> 16) & 0xff] ^ Te2[(t3 >>  8) & 0xff] ^ Te3[t0 & 0xff] ^ rk[25];
   	s2 = Te0[t2 >> 24] ^ Te1[(t3 >> 16) & 0xff] ^ Te2[(t0 >>  8) & 0xff] ^ Te3[t1 & 0xff] ^ rk[26];
   	s3 = Te0[t3 >> 24] ^ Te1[(t0 >> 16) & 0xff] ^ Te2[(t1 >>  8) & 0xff] ^ Te3[t2 & 0xff] ^ rk[27];
	/* round 7: */
   	t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >>  8) & 0xff] ^ Te3[s3 & 0xff] ^ rk[28];
   	t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >>  8) & 0xff] ^ Te3[s0 & 0xff] ^ rk[29];
   	t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >>  8) & 0xff] ^ Te3[s1 & 0xff] ^ rk[30];
   	t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >>  8) & 0xff] ^ Te3[s2 & 0xff] ^ rk[31];
   	/* round 8: */
   	s0 = Te0[t0 >> 24] ^ Te1[(t1 >> 16) & 0xff] ^ Te2[(t2 >>  8) & 0xff] ^ Te3[t3 & 0xff] ^ rk[32];
   	s1 = Te0[t1 >> 24] ^ Te1[(t2 >> 16) & 0xff] ^ Te2[(t3 >>  8) & 0xff] ^ Te3[t0 & 0xff] ^ rk[33];
   	s2 = Te0[t2 >> 24] ^ Te1[(t3 >> 16) & 0xff] ^ Te2[(t0 >>  8) & 0xff] ^ Te3[t1 & 0xff] ^ rk[34];
   	s3 = Te0[t3 >> 24] ^ Te1[(t0 >> 16) & 0xff] ^ Te2[(t1 >>  8) & 0xff] ^ Te3[t2 & 0xff] ^ rk[35];
	/* round 9: */
   	t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >>  8) & 0xff] ^ Te3[s3 & 0xff] ^ rk[36];
   	t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >>  8) & 0xff] ^ Te3[s0 & 0xff] ^ rk[37];
   	t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >>  8) & 0xff] ^ Te3[s1 & 0xff] ^ rk[38];
   	t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >>  8) & 0xff] ^ Te3[s2 & 0xff] ^ rk[39];
    if (key->rounds > 10) {
        /* round 10: */
        s0 = Te0[t0 >> 24] ^ Te1[(t1 >> 16) & 0xff] ^ Te2[(t2 >>  8) & 0xff] ^ Te3[t3 & 0xff] ^ rk[40];
        s1 = Te0[t1 >> 24] ^ Te1[(t2 >> 16) & 0xff] ^ Te2[(t3 >>  8) & 0xff] ^ Te3[t0 & 0xff] ^ rk[41];
        s2 = Te0[t2 >> 24] ^ Te1[(t3 >> 16) & 0xff] ^ Te2[(t0 >>  8) & 0xff] ^ Te3[t1 & 0xff] ^ rk[42];
        s3 = Te0[t3 >> 24] ^ Te1[(t0 >> 16) & 0xff] ^ Te2[(t1 >>  8) & 0xff] ^ Te3[t2 & 0xff] ^ rk[43];
        /* round 11: */
        t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >>  8) & 0xff] ^ Te3[s3 & 0xff] ^ rk[44];
        t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >>  8) & 0xff] ^ Te3[s0 & 0xff] ^ rk[45];
        t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >>  8) & 0xff] ^ Te3[s1 & 0xff] ^ rk[46];
        t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >>  8) & 0xff] ^ Te3[s2 & 0xff] ^ rk[47];
        if (key->rounds > 12) {
            /* round 12: */
            s0 = Te0[t0 >> 24] ^ Te1[(t1 >> 16) & 0xff] ^ Te2[(t2 >>  8) & 0xff] ^ Te3[t3 & 0xff] ^ rk[48];
            s1 = Te0[t1 >> 24] ^ Te1[(t2 >> 16) & 0xff] ^ Te2[(t3 >>  8) & 0xff] ^ Te3[t0 & 0xff] ^ rk[49];
            s2 = Te0[t2 >> 24] ^ Te1[(t3 >> 16) & 0xff] ^ Te2[(t0 >>  8) & 0xff] ^ Te3[t1 & 0xff] ^ rk[50];
            s3 = Te0[t3 >> 24] ^ Te1[(t0 >> 16) & 0xff] ^ Te2[(t1 >>  8) & 0xff] ^ Te3[t2 & 0xff] ^ rk[51];
            /* round 13: */
            t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >>  8) & 0xff] ^ Te3[s3 & 0xff] ^ rk[52];
            t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >>  8) & 0xff] ^ Te3[s0 & 0xff] ^ rk[53];
            t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >>  8) & 0xff] ^ Te3[s1 & 0xff] ^ rk[54];
            t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >>  8) & 0xff] ^ Te3[s2 & 0xff] ^ rk[55];
        }
    }
    rk += key->rounds << 2;
#else  /* !FULL_UNROLL */
    /*
     * Nr - 1 full rounds:
     */
    r = key->rounds >> 1;
    for (;;) {
        t0 =
            Te0[(s0 >> 24)       ] ^
            Te1[(s1 >> 16) & 0xff] ^
            Te2[(s2 >>  8) & 0xff] ^
            Te3[(s3      ) & 0xff] ^
            rk[4];
        t1 =
            Te0[(s1 >> 24)       ] ^
            Te1[(s2 >> 16) & 0xff] ^
            Te2[(s3 >>  8) & 0xff] ^
            Te3[(s0      ) & 0xff] ^
            rk[5];
        t2 =
            Te0[(s2 >> 24)       ] ^
            Te1[(s3 >> 16) & 0xff] ^
            Te2[(s0 >>  8) & 0xff] ^
            Te3[(s1      ) & 0xff] ^
            rk[6];
        t3 =
            Te0[(s3 >> 24)       ] ^
            Te1[(s0 >> 16) & 0xff] ^
            Te2[(s1 >>  8) & 0xff] ^
            Te3[(s2      ) & 0xff] ^
            rk[7];

        rk += 8;
        if (--r == 0) {
            break;
        }

        s0 =
            Te0[(t0 >> 24)       ] ^
            Te1[(t1 >> 16) & 0xff] ^
            Te2[(t2 >>  8) & 0xff] ^
            Te3[(t3      ) & 0xff] ^
            rk[0];
        s1 =
            Te0[(t1 >> 24)       ] ^
            Te1[(t2 >> 16) & 0xff] ^
            Te2[(t3 >>  8) & 0xff] ^
            Te3[(t0      ) & 0xff] ^
            rk[1];
        s2 =
            Te0[(t2 >> 24)       ] ^
            Te1[(t3 >> 16) & 0xff] ^
            Te2[(t0 >>  8) & 0xff] ^
            Te3[(t1      ) & 0xff] ^
            rk[2];
        s3 =
            Te0[(t3 >> 24)       ] ^
            Te1[(t0 >> 16) & 0xff] ^
            Te2[(t1 >>  8) & 0xff] ^
            Te3[(t2      ) & 0xff] ^
            rk[3];
    }
#endif /* ?FULL_UNROLL */
    /*
	 * apply last round and
	 * map cipher state to byte array block:
	 */
	s0 =
		(Te4[(t0 >> 24)       ] & 0xff000000) ^
		(Te4[(t1 >> 16) & 0xff] & 0x00ff0000) ^
		(Te4[(t2 >>  8) & 0xff] & 0x0000ff00) ^
		(Te4[(t3      ) & 0xff] & 0x000000ff) ^
		rk[0];
	PUTU32(out     , s0);
	s1 =
		(Te4[(t1 >> 24)       ] & 0xff000000) ^
		(Te4[(t2 >> 16) & 0xff] & 0x00ff0000) ^
		(Te4[(t3 >>  8) & 0xff] & 0x0000ff00) ^
		(Te4[(t0      ) & 0xff] & 0x000000ff) ^
		rk[1];
	PUTU32(out +  4, s1);
	s2 =
		(Te4[(t2 >> 24)       ] & 0xff000000) ^
		(Te4[(t3 >> 16) & 0xff] & 0x00ff0000) ^
		(Te4[(t0 >>  8) & 0xff] & 0x0000ff00) ^
		(Te4[(t1      ) & 0xff] & 0x000000ff) ^
		rk[2];
	PUTU32(out +  8, s2);
	s3 =
		(Te4[(t3 >> 24)       ] & 0xff000000) ^
		(Te4[(t0 >> 16) & 0xff] & 0x00ff0000) ^
		(Te4[(t1 >>  8) & 0xff] & 0x0000ff00) ^
		(Te4[(t2      ) & 0xff] & 0x000000ff) ^
		rk[3];
	PUTU32(out + 12, s3);
}

/*
 * Decrypt a single block
 * in and out can overlap
 */
void evp_aes_decrypt(const unsigned char *in, unsigned char *out, const AES_KEY *key) 
{

	const u32 *rk;
	u32 s0, s1, s2, s3, t0, t1, t2, t3;
#ifndef FULL_UNROLL
	int r;
#endif /* ?FULL_UNROLL */

	assert(in && out && key);
	rk = key->rd_key;

	/*
	 * map byte array block to cipher state
	 * and add initial round key:
	 */
    s0 = GETU32(in     ) ^ rk[0];
    s1 = GETU32(in +  4) ^ rk[1];
    s2 = GETU32(in +  8) ^ rk[2];
    s3 = GETU32(in + 12) ^ rk[3];
#ifdef FULL_UNROLL
    /* round 1: */
    t0 = Td0[s0 >> 24] ^ Td1[(s3 >> 16) & 0xff] ^ Td2[(s2 >>  8) & 0xff] ^ Td3[s1 & 0xff] ^ rk[ 4];
    t1 = Td0[s1 >> 24] ^ Td1[(s0 >> 16) & 0xff] ^ Td2[(s3 >>  8) & 0xff] ^ Td3[s2 & 0xff] ^ rk[ 5];
    t2 = Td0[s2 >> 24] ^ Td1[(s1 >> 16) & 0xff] ^ Td2[(s0 >>  8) & 0xff] ^ Td3[s3 & 0xff] ^ rk[ 6];
    t3 = Td0[s3 >> 24] ^ Td1[(s2 >> 16) & 0xff] ^ Td2[(s1 >>  8) & 0xff] ^ Td3[s0 & 0xff] ^ rk[ 7];
    /* round 2: */
    s0 = Td0[t0 >> 24] ^ Td1[(t3 >> 16) & 0xff] ^ Td2[(t2 >>  8) & 0xff] ^ Td3[t1 & 0xff] ^ rk[ 8];
    s1 = Td0[t1 >> 24] ^ Td1[(t0 >> 16) & 0xff] ^ Td2[(t3 >>  8) & 0xff] ^ Td3[t2 & 0xff] ^ rk[ 9];
    s2 = Td0[t2 >> 24] ^ Td1[(t1 >> 16) & 0xff] ^ Td2[(t0 >>  8) & 0xff] ^ Td3[t3 & 0xff] ^ rk[10];
    s3 = Td0[t3 >> 24] ^ Td1[(t2 >> 16) & 0xff] ^ Td2[(t1 >>  8) & 0xff] ^ Td3[t0 & 0xff] ^ rk[11];
    /* round 3: */
    t0 = Td0[s0 >> 24] ^ Td1[(s3 >> 16) & 0xff] ^ Td2[(s2 >>  8) & 0xff] ^ Td3[s1 & 0xff] ^ rk[12];
    t1 = Td0[s1 >> 24] ^ Td1[(s0 >> 16) & 0xff] ^ Td2[(s3 >>  8) & 0xff] ^ Td3[s2 & 0xff] ^ rk[13];
    t2 = Td0[s2 >> 24] ^ Td1[(s1 >> 16) & 0xff] ^ Td2[(s0 >>  8) & 0xff] ^ Td3[s3 & 0xff] ^ rk[14];
    t3 = Td0[s3 >> 24] ^ Td1[(s2 >> 16) & 0xff] ^ Td2[(s1 >>  8) & 0xff] ^ Td3[s0 & 0xff] ^ rk[15];
    /* round 4: */
    s0 = Td0[t0 >> 24] ^ Td1[(t3 >> 16) & 0xff] ^ Td2[(t2 >>  8) & 0xff] ^ Td3[t1 & 0xff] ^ rk[16];
    s1 = Td0[t1 >> 24] ^ Td1[(t0 >> 16) & 0xff] ^ Td2[(t3 >>  8) & 0xff] ^ Td3[t2 & 0xff] ^ rk[17];
    s2 = Td0[t2 >> 24] ^ Td1[(t1 >> 16) & 0xff] ^ Td2[(t0 >>  8) & 0xff] ^ Td3[t3 & 0xff] ^ rk[18];
    s3 = Td0[t3 >> 24] ^ Td1[(t2 >> 16) & 0xff] ^ Td2[(t1 >>  8) & 0xff] ^ Td3[t0 & 0xff] ^ rk[19];
    /* round 5: */
    t0 = Td0[s0 >> 24] ^ Td1[(s3 >> 16) & 0xff] ^ Td2[(s2 >>  8) & 0xff] ^ Td3[s1 & 0xff] ^ rk[20];
    t1 = Td0[s1 >> 24] ^ Td1[(s0 >> 16) & 0xff] ^ Td2[(s3 >>  8) & 0xff] ^ Td3[s2 & 0xff] ^ rk[21];
    t2 = Td0[s2 >> 24] ^ Td1[(s1 >> 16) & 0xff] ^ Td2[(s0 >>  8) & 0xff] ^ Td3[s3 & 0xff] ^ rk[22];
    t3 = Td0[s3 >> 24] ^ Td1[(s2 >> 16) & 0xff] ^ Td2[(s1 >>  8) & 0xff] ^ Td3[s0 & 0xff] ^ rk[23];
    /* round 6: */
    s0 = Td0[t0 >> 24] ^ Td1[(t3 >> 16) & 0xff] ^ Td2[(t2 >>  8) & 0xff] ^ Td3[t1 & 0xff] ^ rk[24];
    s1 = Td0[t1 >> 24] ^ Td1[(t0 >> 16) & 0xff] ^ Td2[(t3 >>  8) & 0xff] ^ Td3[t2 & 0xff] ^ rk[25];
    s2 = Td0[t2 >> 24] ^ Td1[(t1 >> 16) & 0xff] ^ Td2[(t0 >>  8) & 0xff] ^ Td3[t3 & 0xff] ^ rk[26];
    s3 = Td0[t3 >> 24] ^ Td1[(t2 >> 16) & 0xff] ^ Td2[(t1 >>  8) & 0xff] ^ Td3[t0 & 0xff] ^ rk[27];
    /* round 7: */
    t0 = Td0[s0 >> 24] ^ Td1[(s3 >> 16) & 0xff] ^ Td2[(s2 >>  8) & 0xff] ^ Td3[s1 & 0xff] ^ rk[28];
    t1 = Td0[s1 >> 24] ^ Td1[(s0 >> 16) & 0xff] ^ Td2[(s3 >>  8) & 0xff] ^ Td3[s2 & 0xff] ^ rk[29];
    t2 = Td0[s2 >> 24] ^ Td1[(s1 >> 16) & 0xff] ^ Td2[(s0 >>  8) & 0xff] ^ Td3[s3 & 0xff] ^ rk[30];
    t3 = Td0[s3 >> 24] ^ Td1[(s2 >> 16) & 0xff] ^ Td2[(s1 >>  8) & 0xff] ^ Td3[s0 & 0xff] ^ rk[31];
    /* round 8: */
    s0 = Td0[t0 >> 24] ^ Td1[(t3 >> 16) & 0xff] ^ Td2[(t2 >>  8) & 0xff] ^ Td3[t1 & 0xff] ^ rk[32];
    s1 = Td0[t1 >> 24] ^ Td1[(t0 >> 16) & 0xff] ^ Td2[(t3 >>  8) & 0xff] ^ Td3[t2 & 0xff] ^ rk[33];
    s2 = Td0[t2 >> 24] ^ Td1[(t1 >> 16) & 0xff] ^ Td2[(t0 >>  8) & 0xff] ^ Td3[t3 & 0xff] ^ rk[34];
    s3 = Td0[t3 >> 24] ^ Td1[(t2 >> 16) & 0xff] ^ Td2[(t1 >>  8) & 0xff] ^ Td3[t0 & 0xff] ^ rk[35];
    /* round 9: */
    t0 = Td0[s0 >> 24] ^ Td1[(s3 >> 16) & 0xff] ^ Td2[(s2 >>  8) & 0xff] ^ Td3[s1 & 0xff] ^ rk[36];
    t1 = Td0[s1 >> 24] ^ Td1[(s0 >> 16) & 0xff] ^ Td2[(s3 >>  8) & 0xff] ^ Td3[s2 & 0xff] ^ rk[37];
    t2 = Td0[s2 >> 24] ^ Td1[(s1 >> 16) & 0xff] ^ Td2[(s0 >>  8) & 0xff] ^ Td3[s3 & 0xff] ^ rk[38];
    t3 = Td0[s3 >> 24] ^ Td1[(s2 >> 16) & 0xff] ^ Td2[(s1 >>  8) & 0xff] ^ Td3[s0 & 0xff] ^ rk[39];
    if (key->rounds > 10) {
        /* round 10: */
        s0 = Td0[t0 >> 24] ^ Td1[(t3 >> 16) & 0xff] ^ Td2[(t2 >>  8) & 0xff] ^ Td3[t1 & 0xff] ^ rk[40];
        s1 = Td0[t1 >> 24] ^ Td1[(t0 >> 16) & 0xff] ^ Td2[(t3 >>  8) & 0xff] ^ Td3[t2 & 0xff] ^ rk[41];
        s2 = Td0[t2 >> 24] ^ Td1[(t1 >> 16) & 0xff] ^ Td2[(t0 >>  8) & 0xff] ^ Td3[t3 & 0xff] ^ rk[42];
        s3 = Td0[t3 >> 24] ^ Td1[(t2 >> 16) & 0xff] ^ Td2[(t1 >>  8) & 0xff] ^ Td3[t0 & 0xff] ^ rk[43];
        /* round 11: */
        t0 = Td0[s0 >> 24] ^ Td1[(s3 >> 16) & 0xff] ^ Td2[(s2 >>  8) & 0xff] ^ Td3[s1 & 0xff] ^ rk[44];
        t1 = Td0[s1 >> 24] ^ Td1[(s0 >> 16) & 0xff] ^ Td2[(s3 >>  8) & 0xff] ^ Td3[s2 & 0xff] ^ rk[45];
        t2 = Td0[s2 >> 24] ^ Td1[(s1 >> 16) & 0xff] ^ Td2[(s0 >>  8) & 0xff] ^ Td3[s3 & 0xff] ^ rk[46];
        t3 = Td0[s3 >> 24] ^ Td1[(s2 >> 16) & 0xff] ^ Td2[(s1 >>  8) & 0xff] ^ Td3[s0 & 0xff] ^ rk[47];
        if (key->rounds > 12) {
            /* round 12: */
            s0 = Td0[t0 >> 24] ^ Td1[(t3 >> 16) & 0xff] ^ Td2[(t2 >>  8) & 0xff] ^ Td3[t1 & 0xff] ^ rk[48];
            s1 = Td0[t1 >> 24] ^ Td1[(t0 >> 16) & 0xff] ^ Td2[(t3 >>  8) & 0xff] ^ Td3[t2 & 0xff] ^ rk[49];
            s2 = Td0[t2 >> 24] ^ Td1[(t1 >> 16) & 0xff] ^ Td2[(t0 >>  8) & 0xff] ^ Td3[t3 & 0xff] ^ rk[50];
            s3 = Td0[t3 >> 24] ^ Td1[(t2 >> 16) & 0xff] ^ Td2[(t1 >>  8) & 0xff] ^ Td3[t0 & 0xff] ^ rk[51];
            /* round 13: */
            t0 = Td0[s0 >> 24] ^ Td1[(s3 >> 16) & 0xff] ^ Td2[(s2 >>  8) & 0xff] ^ Td3[s1 & 0xff] ^ rk[52];
            t1 = Td0[s1 >> 24] ^ Td1[(s0 >> 16) & 0xff] ^ Td2[(s3 >>  8) & 0xff] ^ Td3[s2 & 0xff] ^ rk[53];
            t2 = Td0[s2 >> 24] ^ Td1[(s1 >> 16) & 0xff] ^ Td2[(s0 >>  8) & 0xff] ^ Td3[s3 & 0xff] ^ rk[54];
            t3 = Td0[s3 >> 24] ^ Td1[(s2 >> 16) & 0xff] ^ Td2[(s1 >>  8) & 0xff] ^ Td3[s0 & 0xff] ^ rk[55];
        }
    }
	rk += key->rounds << 2;
#else  /* !FULL_UNROLL */
    /*
     * Nr - 1 full rounds:
     */
    r = key->rounds >> 1;
    for (;;) {
        t0 =
            Td0[(s0 >> 24)       ] ^
            Td1[(s3 >> 16) & 0xff] ^
            Td2[(s2 >>  8) & 0xff] ^
            Td3[(s1      ) & 0xff] ^
            rk[4];
        t1 =
            Td0[(s1 >> 24)       ] ^
            Td1[(s0 >> 16) & 0xff] ^
            Td2[(s3 >>  8) & 0xff] ^
            Td3[(s2      ) & 0xff] ^
            rk[5];
        t2 =
            Td0[(s2 >> 24)       ] ^
            Td1[(s1 >> 16) & 0xff] ^
            Td2[(s0 >>  8) & 0xff] ^
            Td3[(s3      ) & 0xff] ^
            rk[6];
        t3 =
            Td0[(s3 >> 24)       ] ^
            Td1[(s2 >> 16) & 0xff] ^
            Td2[(s1 >>  8) & 0xff] ^
            Td3[(s0      ) & 0xff] ^
            rk[7];

        rk += 8;
        if (--r == 0) {
            break;
        }

        s0 =
            Td0[(t0 >> 24)       ] ^
            Td1[(t3 >> 16) & 0xff] ^
            Td2[(t2 >>  8) & 0xff] ^
            Td3[(t1      ) & 0xff] ^
            rk[0];
        s1 =
            Td0[(t1 >> 24)       ] ^
            Td1[(t0 >> 16) & 0xff] ^
            Td2[(t3 >>  8) & 0xff] ^
            Td3[(t2      ) & 0xff] ^
            rk[1];
        s2 =
            Td0[(t2 >> 24)       ] ^
            Td1[(t1 >> 16) & 0xff] ^
            Td2[(t0 >>  8) & 0xff] ^
            Td3[(t3      ) & 0xff] ^
            rk[2];
        s3 =
            Td0[(t3 >> 24)       ] ^
            Td1[(t2 >> 16) & 0xff] ^
            Td2[(t1 >>  8) & 0xff] ^
            Td3[(t0      ) & 0xff] ^
            rk[3];
    }
#endif /* ?FULL_UNROLL */
    /*
	 * apply last round and
	 * map cipher state to byte array block:
	 */
   	s0 =
   		(Td4[(t0 >> 24)       ] & 0xff000000) ^
   		(Td4[(t3 >> 16) & 0xff] & 0x00ff0000) ^
   		(Td4[(t2 >>  8) & 0xff] & 0x0000ff00) ^
   		(Td4[(t1      ) & 0xff] & 0x000000ff) ^
   		rk[0];
	PUTU32(out     , s0);
   	s1 =
   		(Td4[(t1 >> 24)       ] & 0xff000000) ^
   		(Td4[(t0 >> 16) & 0xff] & 0x00ff0000) ^
   		(Td4[(t3 >>  8) & 0xff] & 0x0000ff00) ^
   		(Td4[(t2      ) & 0xff] & 0x000000ff) ^
   		rk[1];
	PUTU32(out +  4, s1);
   	s2 =
   		(Td4[(t2 >> 24)       ] & 0xff000000) ^
   		(Td4[(t1 >> 16) & 0xff] & 0x00ff0000) ^
   		(Td4[(t0 >>  8) & 0xff] & 0x0000ff00) ^
   		(Td4[(t3      ) & 0xff] & 0x000000ff) ^
   		rk[2];
	PUTU32(out +  8, s2);
   	s3 =
   		(Td4[(t3 >> 24)       ] & 0xff000000) ^
   		(Td4[(t2 >> 16) & 0xff] & 0x00ff0000) ^
   		(Td4[(t1 >>  8) & 0xff] & 0x0000ff00) ^
   		(Td4[(t0      ) & 0xff] & 0x000000ff) ^
   		rk[3];
	PUTU32(out + 12, s3);
}

/**
 * Expand the cipher key into the encryption key schedule.
 */
int AES_set_encrypt_key(const unsigned char *userKey, const int bits, AES_KEY *key) 
{

	u32 *rk;
   	int i = 0;
	u32 temp;

	if (!userKey || !key)
		return -1;
	if (bits != 128 && bits != 192 && bits != 256)
		return -2;

	rk = key->rd_key;

	if (bits==128)
		key->rounds = 10;
	else if (bits==192)
		key->rounds = 12;
	else
		key->rounds = 14;

	rk[0] = GETU32(userKey     );
	rk[1] = GETU32(userKey +  4);
	rk[2] = GETU32(userKey +  8);
	rk[3] = GETU32(userKey + 12);
	if (bits == 128) {
		while (1) {
			temp  = rk[3];
			rk[4] = rk[0] ^
				(Te4[(temp >> 16) & 0xff] & 0xff000000) ^
				(Te4[(temp >>  8) & 0xff] & 0x00ff0000) ^
				(Te4[(temp      ) & 0xff] & 0x0000ff00) ^
				(Te4[(temp >> 24)       ] & 0x000000ff) ^
				rcon[i];
			rk[5] = rk[1] ^ rk[4];
			rk[6] = rk[2] ^ rk[5];
			rk[7] = rk[3] ^ rk[6];
			if (++i == 10) {
				return 0;
			}
			rk += 4;
		}
	}
	rk[4] = GETU32(userKey + 16);
	rk[5] = GETU32(userKey + 20);
	if (bits == 192) {
		while (1) {
			temp = rk[ 5];
			rk[ 6] = rk[ 0] ^
				(Te4[(temp >> 16) & 0xff] & 0xff000000) ^
				(Te4[(temp >>  8) & 0xff] & 0x00ff0000) ^
				(Te4[(temp      ) & 0xff] & 0x0000ff00) ^
				(Te4[(temp >> 24)       ] & 0x000000ff) ^
				rcon[i];
			rk[ 7] = rk[ 1] ^ rk[ 6];
			rk[ 8] = rk[ 2] ^ rk[ 7];
			rk[ 9] = rk[ 3] ^ rk[ 8];
			if (++i == 8) {
				return 0;
			}
			rk[10] = rk[ 4] ^ rk[ 9];
			rk[11] = rk[ 5] ^ rk[10];
			rk += 6;
		}
	}
	rk[6] = GETU32(userKey + 24);
	rk[7] = GETU32(userKey + 28);
	if (bits == 256) {
		while (1) {
			temp = rk[ 7];
			rk[ 8] = rk[ 0] ^
				(Te4[(temp >> 16) & 0xff] & 0xff000000) ^
				(Te4[(temp >>  8) & 0xff] & 0x00ff0000) ^
				(Te4[(temp      ) & 0xff] & 0x0000ff00) ^
				(Te4[(temp >> 24)       ] & 0x000000ff) ^
				rcon[i];
			rk[ 9] = rk[ 1] ^ rk[ 8];
			rk[10] = rk[ 2] ^ rk[ 9];
			rk[11] = rk[ 3] ^ rk[10];
			if (++i == 7) {
				return 0;
			}
			temp = rk[11];
			rk[12] = rk[ 4] ^
				(Te4[(temp >> 24)       ] & 0xff000000) ^
				(Te4[(temp >> 16) & 0xff] & 0x00ff0000) ^
				(Te4[(temp >>  8) & 0xff] & 0x0000ff00) ^
				(Te4[(temp      ) & 0xff] & 0x000000ff);
			rk[13] = rk[ 5] ^ rk[12];
			rk[14] = rk[ 6] ^ rk[13];
			rk[15] = rk[ 7] ^ rk[14];

			rk += 8;
        	}
	}
	return 0;   /*Success
*/
}

void evp_aes_cbc_encrypt(const unsigned char *in, unsigned char *out,
		     const unsigned long length, const AES_KEY *key,
		     unsigned char *ivec, const int enc) 
{

	unsigned long n;
	unsigned long len = length;
	unsigned char tmp[AES_BLOCK_SIZE];
	const unsigned char *iv = ivec;

	assert(in && out && key && ivec);
	assert((AES_ENCRYPT == enc)||(AES_DECRYPT == enc));

	if (AES_ENCRYPT == enc) {
		while (len >= AES_BLOCK_SIZE) {
			for(n=0; n < AES_BLOCK_SIZE; ++n)
				out[n] = in[n] ^ iv[n];
			evp_aes_encrypt(out, out, key);
			iv = out;
			len -= AES_BLOCK_SIZE;
			in += AES_BLOCK_SIZE;
			out += AES_BLOCK_SIZE;
		}
		if (len) {
			for(n=0; n < len; ++n)
				out[n] = in[n] ^ iv[n];
			for(n=len; n < AES_BLOCK_SIZE; ++n)
				out[n] = iv[n];
			evp_aes_encrypt(out, out, key);
			iv = out;
		}
		memcpy(ivec,iv,AES_BLOCK_SIZE);
	} else if (in != out) {
		while (len >= AES_BLOCK_SIZE) {
			evp_aes_decrypt(in, out, key);
			for(n=0; n < AES_BLOCK_SIZE; ++n)
				out[n] ^= iv[n];
			iv = in;
			len -= AES_BLOCK_SIZE;
			in  += AES_BLOCK_SIZE;
			out += AES_BLOCK_SIZE;
		}
		if (len) {
			evp_aes_decrypt(in,tmp,key);
			for(n=0; n < len; ++n)
				out[n] = tmp[n] ^ iv[n];
			iv = in;
		}
		memcpy(ivec,iv,AES_BLOCK_SIZE);
	} else {
		while (len >= AES_BLOCK_SIZE) {
			memcpy(tmp, in, AES_BLOCK_SIZE);
			evp_aes_decrypt(in, out, key);
			for(n=0; n < AES_BLOCK_SIZE; ++n)
				out[n] ^= ivec[n];
			memcpy(ivec, tmp, AES_BLOCK_SIZE);
			len -= AES_BLOCK_SIZE;
			in += AES_BLOCK_SIZE;
			out += AES_BLOCK_SIZE;
		}
		if (len) {
			memcpy(tmp, in, AES_BLOCK_SIZE);
			evp_aes_decrypt(tmp, out, key);
			for(n=0; n < len; ++n)
				out[n] ^= ivec[n];
			for(n=len; n < AES_BLOCK_SIZE; ++n)
				out[n] = tmp[n];
			memcpy(ivec, tmp, AES_BLOCK_SIZE);
		}
	}
}

/**
 * Expand the cipher key into the decryption key schedule.
 */
int AES_set_decrypt_key(const unsigned char *userKey, const int bits, AES_KEY *key) 
{

        u32 *rk;
	int i, j, status;
	u32 temp;

	/* first, start with an encryption schedule */
	status = AES_set_encrypt_key(userKey, bits, key);
	if (status < 0)
		return status;

	rk = key->rd_key;

	/* invert the order of the round keys: */
	for (i = 0, j = 4*(key->rounds); i < j; i += 4, j -= 4) {
		temp = rk[i    ]; rk[i    ] = rk[j    ]; rk[j    ] = temp;
		temp = rk[i + 1]; rk[i + 1] = rk[j + 1]; rk[j + 1] = temp;
		temp = rk[i + 2]; rk[i + 2] = rk[j + 2]; rk[j + 2] = temp;
		temp = rk[i + 3]; rk[i + 3] = rk[j + 3]; rk[j + 3] = temp;
	}
	/* apply the inverse MixColumn transform to all round keys but the first and the last: */
	for (i = 1; i < (key->rounds); i++) {
		rk += 4;
		rk[0] =
			Td0[Te4[(rk[0] >> 24)       ] & 0xff] ^
			Td1[Te4[(rk[0] >> 16) & 0xff] & 0xff] ^
			Td2[Te4[(rk[0] >>  8) & 0xff] & 0xff] ^
			Td3[Te4[(rk[0]      ) & 0xff] & 0xff];
		rk[1] =
			Td0[Te4[(rk[1] >> 24)       ] & 0xff] ^
			Td1[Te4[(rk[1] >> 16) & 0xff] & 0xff] ^
			Td2[Te4[(rk[1] >>  8) & 0xff] & 0xff] ^
			Td3[Te4[(rk[1]      ) & 0xff] & 0xff];
		rk[2] =
			Td0[Te4[(rk[2] >> 24)       ] & 0xff] ^
			Td1[Te4[(rk[2] >> 16) & 0xff] & 0xff] ^
			Td2[Te4[(rk[2] >>  8) & 0xff] & 0xff] ^
			Td3[Te4[(rk[2]      ) & 0xff] & 0xff];
		rk[3] =
			Td0[Te4[(rk[3] >> 24)       ] & 0xff] ^
			Td1[Te4[(rk[3] >> 16) & 0xff] & 0xff] ^
			Td2[Te4[(rk[3] >>  8) & 0xff] & 0xff] ^
			Td3[Te4[(rk[3]      ) & 0xff] & 0xff];
	}
	return 0;
}

int EVP_aes_128_cbc()
{
	return 128; /*aes_128_cbc
*/
}

int EVP_EncryptInit(EVP_CIPHER_CTX *ctx, int type, unsigned char *key, unsigned char *iv)
{
	int ret;

	memset(ctx, 0x00, sizeof(EVP_CIPHER_CTX));

	ctx->flag = 1;   /*Init ok.
*/
	ctx->type = type;
	ctx->encrypt = 1;  /*Do Encrypt
*/
	

	memcpy(ctx->key, key, 16);
	memcpy(ctx->iv, iv, 16);
	
	ret = AES_set_encrypt_key(key, 128, &ctx->aesKey);

	return ret;
}

int EVP_EncryptUpdate(EVP_CIPHER_CTX *ctx, unsigned char *outbuf, int *outlen, unsigned char *inbuf, int inlen)
{
	int encryptSize = 0;

	if ((ctx->flag == 0) || (inlen == 0) || (ctx->encrypt == 0))
		return 0;  /*Failed, ctx not been initializzd or input size is empty.
*/

	while (inlen >= AES_BLOCK_SIZE)
	{
		evp_aes_cbc_encrypt(inbuf + encryptSize, outbuf + encryptSize, AES_BLOCK_SIZE, &ctx->aesKey, ctx->iv, ctx->encrypt);
		encryptSize += AES_BLOCK_SIZE;
		inlen       -= AES_BLOCK_SIZE;
	}

	if (inlen == 0)
	{
		*outlen = encryptSize;
		memset(ctx->buffer, AES_BLOCK_SIZE, AES_BLOCK_SIZE);
		ctx->bufferlen = AES_BLOCK_SIZE;
	}
	else
	{
		*outlen = encryptSize;
		memcpy(ctx->buffer, inbuf + encryptSize, inlen);
		memset(ctx->buffer + inlen, AES_BLOCK_SIZE - inlen, AES_BLOCK_SIZE - inlen);
		ctx->bufferlen = AES_BLOCK_SIZE;
	}

	return 1; /*Success
*/
}

int EVP_EncryptFinal(EVP_CIPHER_CTX *ctx, unsigned char *outbuf, int *outlen)
{
	if ((ctx->flag == 0) || (ctx->bufferlen == 0) || (ctx->encrypt == 0))
	{
		*outlen = 0;
		return 0; /*Failed, ctx not been initialized or buffer is empty.
*/
	}

	*outlen = AES_BLOCK_SIZE;
	evp_aes_cbc_encrypt(ctx->buffer, outbuf, AES_BLOCK_SIZE, &ctx->aesKey, ctx->iv, ctx->encrypt);

	return 1; /*Success
*/
}

int EVP_DecryptInit(EVP_CIPHER_CTX *ctx, int type, unsigned char *key, unsigned char *iv)
{
	int ret;

	memset(ctx, 0x00, sizeof(EVP_CIPHER_CTX));

	ctx->flag = 1;   /*Init ok.
*/
	ctx->type = type;
	ctx->encrypt = 0;  /*Do Decrypt
*/
	
	memcpy(ctx->key, key, 16);
	memcpy(ctx->iv, iv, 16);
	
	ret = AES_set_decrypt_key(key, 128, &ctx->aesKey);

	return ret;
}

int EVP_DecryptUpdate(EVP_CIPHER_CTX *ctx, unsigned char *outbuf, int *outlen, unsigned char *inbuf, int inlen)
{
	int decryptSize = 0;

	if ((ctx->flag == 0) || (inlen == 0) || (ctx->encrypt == 1))
		return 0;  /*Failed, ctx not been initializzd or input size is empty.
*/

	while (inlen >= AES_BLOCK_SIZE)
	{
		if (inlen <= AES_BLOCK_SIZE)
			break;

		evp_aes_cbc_encrypt(inbuf + decryptSize, outbuf + decryptSize, AES_BLOCK_SIZE, &ctx->aesKey, ctx->iv, ctx->encrypt);
		decryptSize += AES_BLOCK_SIZE;
		inlen       -= AES_BLOCK_SIZE;
	}

	if (inlen == AES_BLOCK_SIZE)
	{
		ctx->bufferlen = inlen;
		evp_aes_cbc_encrypt(inbuf + decryptSize, ctx->buffer, AES_BLOCK_SIZE, &ctx->aesKey, ctx->iv, ctx->encrypt);
	}
	*outlen = decryptSize;

	return 1; /*Success.
*/
}

int EVP_DecryptFinal(EVP_CIPHER_CTX *ctx, unsigned char *outbuf, int *outlen)
{
	unsigned char lastchar = 0;
	int datasize = 0;

	if ((ctx->flag == 0) || (ctx->bufferlen == 0) || (ctx->encrypt == 1))
	{
		*outlen = 0;
		return 0; /*Failed, ctx not been initialized or buffer is empty.
*/
	}

	lastchar = ctx->buffer[AES_BLOCK_SIZE - 1];
	if (lastchar < AES_BLOCK_SIZE)
	{
		datasize = AES_BLOCK_SIZE - lastchar;
		memcpy(outbuf, ctx->buffer, datasize);
	}

	*outlen = datasize;

	return 1; /*Success
*/
}



/*  Wifi-Simple-Configure                               *
 *  Encryption algorithm : AES-128-CBC per FIPS 197     */

/* 
 * WscEncryptData
 * @ plainText: input data
 * @ ptx_len:
 * @ key: KeyWrapKey derived from KDK, len=aes-128-bit (in bytes)
 * @ iv: a random iv, len=aes-128-bit (in bytes)
 * @ cipherText: output data
 * @ ctx_len:
 */
void WscEncryptData(
    unsigned char *plainText,   int ptx_len,
    unsigned char *key, unsigned char *iv,
    unsigned char *cipherText,  int *ctx_len)
{
/*    EVP_CIPHER_CTX ctx;
*/
	EVP_CIPHER_CTX *pctx = NULL;
    int bufLen = 1024;
/*    unsigned char outBuf[1024];
*/
	unsigned char *outBuf = NULL;
    int outLen, currentLength;
    
    /*block size = 1024 bytes - 128 bits, 
*/
    /*leave 128 bits at the end to accommodate any possible padding 
*/
    /*and avoid a buffer overflow
*/
    int blockSize = bufLen - AES_BLOCK_SIZE; 
    unsigned char *bufPtr=NULL;
    int data_len;

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&pctx, sizeof(EVP_CIPHER_CTX));
	if (pctx == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		goto LabelErr;
	}
	os_alloc_mem(NULL, (UCHAR **)&outBuf, 1024);
	if (outBuf == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		goto LabelErr;
	}

    /* init buffer
*/
    bufPtr = plainText;
    data_len = ptx_len;

    if(0 != EVP_EncryptInit(pctx, EVP_aes_128_cbc(), key, iv))
    {
        DBGPRINT(RT_DEBUG_INFO, ("WscEncryptData: EncryptInit failed\n"));
    }

    *ctx_len = 0;
    while(data_len)
    {
        if(data_len > blockSize)
            currentLength = blockSize;
        else
            currentLength = data_len;

        if(0 == EVP_EncryptUpdate(pctx, outBuf, &outLen, bufPtr, currentLength))
        {
            DBGPRINT(RT_DEBUG_INFO, ("WscEncryptData: EncryptUpdate failed\n")); 
        }
        
        /* fill in output cipherText
*/
        memcpy(cipherText, outBuf, outLen);
        cipherText += outLen;
        *ctx_len += outLen;
        
        bufPtr += currentLength;
        data_len -= currentLength;
    }

    
    if(0 == EVP_EncryptFinal(pctx, outBuf, &outLen))
    {
        DBGPRINT(RT_DEBUG_INFO, ("WscEncryptData: EncryptFinal failed\n"));
    }

    /* fill in output cipherText
*/
    memcpy(cipherText, outBuf, outLen);
    *ctx_len += outLen;

LabelErr:
	if (pctx != NULL)
		os_free_mem(NULL, pctx);
	if (outBuf != NULL)
		os_free_mem(NULL, outBuf);
}


/* WscDecryptData
 * @ cipherText: input data
 * @ ctx_len:
 * @ key: KeyWrapKey derived from KDK, len=aes-128-bit (in bytes)
 * @ iv: a random iv, len=aes-128-bit (in bytes)
 * @ plainText: output data
 * @ ptx_len:
 */
void WscDecryptData(
    unsigned char *cipherText,  int ctx_len, 
    unsigned char *key, unsigned char *iv,
    unsigned char *plainText,   int *ptx_len)
{
/*    EVP_CIPHER_CTX ctx;
*/
	EVP_CIPHER_CTX *pctx = NULL;
    int bufLen = 1024;
/*    unsigned char  outBuf[1024];
*/
	unsigned char *outBuf = NULL;
    int outLen = 0, currentLength;
    
    /*block size = 1024 bytes - 128 bits, 
*/
    /*leave 128 bits at the end to accommodate any possible padding 
*/
    /*and avoid a buffer overflow
*/
    int blockSize = bufLen - AES_BLOCK_SIZE;
    unsigned char *bufPtr=NULL;
    int data_len;

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&pctx, sizeof(EVP_CIPHER_CTX));
	if (pctx == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		goto LabelErr;
	}
	os_alloc_mem(NULL, (UCHAR **)&outBuf, 1024);
	if (outBuf == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		goto LabelErr;
	}

    /* init buffer
*/
    bufPtr = cipherText;
    data_len = ctx_len;

    if(0 != EVP_DecryptInit(pctx, EVP_aes_128_cbc(), key, iv))
    {
        DBGPRINT(RT_DEBUG_INFO, ("WscEncryptData: DecryptInit failed\n"));
    }

    *ptx_len=0;
    while(data_len)
    {
        if(data_len > blockSize)
            currentLength = blockSize;
        else
            currentLength = data_len;

        if(0 == EVP_DecryptUpdate(pctx, outBuf, &outLen, bufPtr, currentLength))
        {
            DBGPRINT(RT_DEBUG_INFO, ("WscDecryptData: DecryptUpdate failed\n"));
        }


        /* fill in output plainText
*/
        memcpy(plainText, outBuf, outLen);
        plainText += outLen;
        *ptx_len += outLen;
        
        bufPtr += currentLength;
        data_len -= currentLength;
    }


    if(0 == EVP_DecryptFinal(pctx, outBuf, &outLen))
    {
        DBGPRINT(RT_DEBUG_INFO, ("WscDecryptData: DecryptFinal failed\n"));
    }

    /* fill in output plainText
*/
    memcpy(plainText, outBuf, outLen);
    *ptx_len += outLen;

LabelErr:
	if (pctx != NULL)
		os_free_mem(NULL, pctx);
	if (outBuf != NULL)
		os_free_mem(NULL, outBuf);
}

/* =========================  AES En/Decryption ========================== */

/* forward S-box */
static uint32 FSb[256] =
{
	0x63, 0x7C,	0x77, 0x7B,	0xF2, 0x6B,	0x6F, 0xC5,
	0x30, 0x01,	0x67, 0x2B,	0xFE, 0xD7,	0xAB, 0x76,
	0xCA, 0x82,	0xC9, 0x7D,	0xFA, 0x59,	0x47, 0xF0,
	0xAD, 0xD4,	0xA2, 0xAF,	0x9C, 0xA4,	0x72, 0xC0,
	0xB7, 0xFD,	0x93, 0x26,	0x36, 0x3F,	0xF7, 0xCC,
	0x34, 0xA5,	0xE5, 0xF1,	0x71, 0xD8,	0x31, 0x15,
	0x04, 0xC7,	0x23, 0xC3,	0x18, 0x96,	0x05, 0x9A,
	0x07, 0x12,	0x80, 0xE2,	0xEB, 0x27,	0xB2, 0x75,
	0x09, 0x83,	0x2C, 0x1A,	0x1B, 0x6E,	0x5A, 0xA0,
	0x52, 0x3B,	0xD6, 0xB3,	0x29, 0xE3,	0x2F, 0x84,
	0x53, 0xD1,	0x00, 0xED,	0x20, 0xFC,	0xB1, 0x5B,
	0x6A, 0xCB,	0xBE, 0x39,	0x4A, 0x4C,	0x58, 0xCF,
	0xD0, 0xEF,	0xAA, 0xFB,	0x43, 0x4D,	0x33, 0x85,
	0x45, 0xF9,	0x02, 0x7F,	0x50, 0x3C,	0x9F, 0xA8,
	0x51, 0xA3,	0x40, 0x8F,	0x92, 0x9D,	0x38, 0xF5,
	0xBC, 0xB6,	0xDA, 0x21,	0x10, 0xFF,	0xF3, 0xD2,
	0xCD, 0x0C,	0x13, 0xEC,	0x5F, 0x97,	0x44, 0x17,
	0xC4, 0xA7,	0x7E, 0x3D,	0x64, 0x5D,	0x19, 0x73,
	0x60, 0x81,	0x4F, 0xDC,	0x22, 0x2A,	0x90, 0x88,
	0x46, 0xEE,	0xB8, 0x14,	0xDE, 0x5E,	0x0B, 0xDB,
	0xE0, 0x32,	0x3A, 0x0A,	0x49, 0x06,	0x24, 0x5C,
	0xC2, 0xD3,	0xAC, 0x62,	0x91, 0x95,	0xE4, 0x79,
	0xE7, 0xC8,	0x37, 0x6D,	0x8D, 0xD5,	0x4E, 0xA9,
	0x6C, 0x56,	0xF4, 0xEA,	0x65, 0x7A,	0xAE, 0x08,
	0xBA, 0x78,	0x25, 0x2E,	0x1C, 0xA6,	0xB4, 0xC6,
	0xE8, 0xDD,	0x74, 0x1F,	0x4B, 0xBD,	0x8B, 0x8A,
	0x70, 0x3E,	0xB5, 0x66,	0x48, 0x03,	0xF6, 0x0E,
	0x61, 0x35,	0x57, 0xB9,	0x86, 0xC1,	0x1D, 0x9E,
	0xE1, 0xF8,	0x98, 0x11,	0x69, 0xD9,	0x8E, 0x94,
	0x9B, 0x1E,	0x87, 0xE9,	0xCE, 0x55,	0x28, 0xDF,
	0x8C, 0xA1,	0x89, 0x0D,	0xBF, 0xE6,	0x42, 0x68,
	0x41, 0x99,	0x2D, 0x0F,	0xB0, 0x54,	0xBB, 0x16
};

/* forward table */
#define	FT \
\
	V(C6,63,63,A5),	V(F8,7C,7C,84),	V(EE,77,77,99),	V(F6,7B,7B,8D),	\
	V(FF,F2,F2,0D),	V(D6,6B,6B,BD),	V(DE,6F,6F,B1),	V(91,C5,C5,54),	\
	V(60,30,30,50),	V(02,01,01,03),	V(CE,67,67,A9),	V(56,2B,2B,7D),	\
	V(E7,FE,FE,19),	V(B5,D7,D7,62),	V(4D,AB,AB,E6),	V(EC,76,76,9A),	\
	V(8F,CA,CA,45),	V(1F,82,82,9D),	V(89,C9,C9,40),	V(FA,7D,7D,87),	\
	V(EF,FA,FA,15),	V(B2,59,59,EB),	V(8E,47,47,C9),	V(FB,F0,F0,0B),	\
	V(41,AD,AD,EC),	V(B3,D4,D4,67),	V(5F,A2,A2,FD),	V(45,AF,AF,EA),	\
	V(23,9C,9C,BF),	V(53,A4,A4,F7),	V(E4,72,72,96),	V(9B,C0,C0,5B),	\
	V(75,B7,B7,C2),	V(E1,FD,FD,1C),	V(3D,93,93,AE),	V(4C,26,26,6A),	\
	V(6C,36,36,5A),	V(7E,3F,3F,41),	V(F5,F7,F7,02),	V(83,CC,CC,4F),	\
	V(68,34,34,5C),	V(51,A5,A5,F4),	V(D1,E5,E5,34),	V(F9,F1,F1,08),	\
	V(E2,71,71,93),	V(AB,D8,D8,73),	V(62,31,31,53),	V(2A,15,15,3F),	\
	V(08,04,04,0C),	V(95,C7,C7,52),	V(46,23,23,65),	V(9D,C3,C3,5E),	\
	V(30,18,18,28),	V(37,96,96,A1),	V(0A,05,05,0F),	V(2F,9A,9A,B5),	\
	V(0E,07,07,09),	V(24,12,12,36),	V(1B,80,80,9B),	V(DF,E2,E2,3D),	\
	V(CD,EB,EB,26),	V(4E,27,27,69),	V(7F,B2,B2,CD),	V(EA,75,75,9F),	\
	V(12,09,09,1B),	V(1D,83,83,9E),	V(58,2C,2C,74),	V(34,1A,1A,2E),	\
	V(36,1B,1B,2D),	V(DC,6E,6E,B2),	V(B4,5A,5A,EE),	V(5B,A0,A0,FB),	\
	V(A4,52,52,F6),	V(76,3B,3B,4D),	V(B7,D6,D6,61),	V(7D,B3,B3,CE),	\
	V(52,29,29,7B),	V(DD,E3,E3,3E),	V(5E,2F,2F,71),	V(13,84,84,97),	\
	V(A6,53,53,F5),	V(B9,D1,D1,68),	V(00,00,00,00),	V(C1,ED,ED,2C),	\
	V(40,20,20,60),	V(E3,FC,FC,1F),	V(79,B1,B1,C8),	V(B6,5B,5B,ED),	\
	V(D4,6A,6A,BE),	V(8D,CB,CB,46),	V(67,BE,BE,D9),	V(72,39,39,4B),	\
	V(94,4A,4A,DE),	V(98,4C,4C,D4),	V(B0,58,58,E8),	V(85,CF,CF,4A),	\
	V(BB,D0,D0,6B),	V(C5,EF,EF,2A),	V(4F,AA,AA,E5),	V(ED,FB,FB,16),	\
	V(86,43,43,C5),	V(9A,4D,4D,D7),	V(66,33,33,55),	V(11,85,85,94),	\
	V(8A,45,45,CF),	V(E9,F9,F9,10),	V(04,02,02,06),	V(FE,7F,7F,81),	\
	V(A0,50,50,F0),	V(78,3C,3C,44),	V(25,9F,9F,BA),	V(4B,A8,A8,E3),	\
	V(A2,51,51,F3),	V(5D,A3,A3,FE),	V(80,40,40,C0),	V(05,8F,8F,8A),	\
	V(3F,92,92,AD),	V(21,9D,9D,BC),	V(70,38,38,48),	V(F1,F5,F5,04),	\
	V(63,BC,BC,DF),	V(77,B6,B6,C1),	V(AF,DA,DA,75),	V(42,21,21,63),	\
	V(20,10,10,30),	V(E5,FF,FF,1A),	V(FD,F3,F3,0E),	V(BF,D2,D2,6D),	\
	V(81,CD,CD,4C),	V(18,0C,0C,14),	V(26,13,13,35),	V(C3,EC,EC,2F),	\
	V(BE,5F,5F,E1),	V(35,97,97,A2),	V(88,44,44,CC),	V(2E,17,17,39),	\
	V(93,C4,C4,57),	V(55,A7,A7,F2),	V(FC,7E,7E,82),	V(7A,3D,3D,47),	\
	V(C8,64,64,AC),	V(BA,5D,5D,E7),	V(32,19,19,2B),	V(E6,73,73,95),	\
	V(C0,60,60,A0),	V(19,81,81,98),	V(9E,4F,4F,D1),	V(A3,DC,DC,7F),	\
	V(44,22,22,66),	V(54,2A,2A,7E),	V(3B,90,90,AB),	V(0B,88,88,83),	\
	V(8C,46,46,CA),	V(C7,EE,EE,29),	V(6B,B8,B8,D3),	V(28,14,14,3C),	\
	V(A7,DE,DE,79),	V(BC,5E,5E,E2),	V(16,0B,0B,1D),	V(AD,DB,DB,76),	\
	V(DB,E0,E0,3B),	V(64,32,32,56),	V(74,3A,3A,4E),	V(14,0A,0A,1E),	\
	V(92,49,49,DB),	V(0C,06,06,0A),	V(48,24,24,6C),	V(B8,5C,5C,E4),	\
	V(9F,C2,C2,5D),	V(BD,D3,D3,6E),	V(43,AC,AC,EF),	V(C4,62,62,A6),	\
	V(39,91,91,A8),	V(31,95,95,A4),	V(D3,E4,E4,37),	V(F2,79,79,8B),	\
	V(D5,E7,E7,32),	V(8B,C8,C8,43),	V(6E,37,37,59),	V(DA,6D,6D,B7),	\
	V(01,8D,8D,8C),	V(B1,D5,D5,64),	V(9C,4E,4E,D2),	V(49,A9,A9,E0),	\
	V(D8,6C,6C,B4),	V(AC,56,56,FA),	V(F3,F4,F4,07),	V(CF,EA,EA,25),	\
	V(CA,65,65,AF),	V(F4,7A,7A,8E),	V(47,AE,AE,E9),	V(10,08,08,18),	\
	V(6F,BA,BA,D5),	V(F0,78,78,88),	V(4A,25,25,6F),	V(5C,2E,2E,72),	\
	V(38,1C,1C,24),	V(57,A6,A6,F1),	V(73,B4,B4,C7),	V(97,C6,C6,51),	\
	V(CB,E8,E8,23),	V(A1,DD,DD,7C),	V(E8,74,74,9C),	V(3E,1F,1F,21),	\
	V(96,4B,4B,DD),	V(61,BD,BD,DC),	V(0D,8B,8B,86),	V(0F,8A,8A,85),	\
	V(E0,70,70,90),	V(7C,3E,3E,42),	V(71,B5,B5,C4),	V(CC,66,66,AA),	\
	V(90,48,48,D8),	V(06,03,03,05),	V(F7,F6,F6,01),	V(1C,0E,0E,12),	\
	V(C2,61,61,A3),	V(6A,35,35,5F),	V(AE,57,57,F9),	V(69,B9,B9,D0),	\
	V(17,86,86,91),	V(99,C1,C1,58),	V(3A,1D,1D,27),	V(27,9E,9E,B9),	\
	V(D9,E1,E1,38),	V(EB,F8,F8,13),	V(2B,98,98,B3),	V(22,11,11,33),	\
	V(D2,69,69,BB),	V(A9,D9,D9,70),	V(07,8E,8E,89),	V(33,94,94,A7),	\
	V(2D,9B,9B,B6),	V(3C,1E,1E,22),	V(15,87,87,92),	V(C9,E9,E9,20),	\
	V(87,CE,CE,49),	V(AA,55,55,FF),	V(50,28,28,78),	V(A5,DF,DF,7A),	\
	V(03,8C,8C,8F),	V(59,A1,A1,F8),	V(09,89,89,80),	V(1A,0D,0D,17),	\
	V(65,BF,BF,DA),	V(D7,E6,E6,31),	V(84,42,42,C6),	V(D0,68,68,B8),	\
	V(82,41,41,C3),	V(29,99,99,B0),	V(5A,2D,2D,77),	V(1E,0F,0F,11),	\
	V(7B,B0,B0,CB),	V(A8,54,54,FC),	V(6D,BB,BB,D6),	V(2C,16,16,3A)

#define	V(a,b,c,d) 0x##a##b##c##d
static uint32 FT0[256] = { FT };
#undef V

#define	V(a,b,c,d) 0x##d##a##b##c
static uint32 FT1[256] = { FT };
#undef V

#define	V(a,b,c,d) 0x##c##d##a##b
static uint32 FT2[256] = { FT };
#undef V

#define	V(a,b,c,d) 0x##b##c##d##a
static uint32 FT3[256] = { FT };
#undef V

#undef FT

/* reverse S-box */

static uint32 RSb[256] =
{
	0x52, 0x09,	0x6A, 0xD5,	0x30, 0x36,	0xA5, 0x38,
	0xBF, 0x40,	0xA3, 0x9E,	0x81, 0xF3,	0xD7, 0xFB,
	0x7C, 0xE3,	0x39, 0x82,	0x9B, 0x2F,	0xFF, 0x87,
	0x34, 0x8E,	0x43, 0x44,	0xC4, 0xDE,	0xE9, 0xCB,
	0x54, 0x7B,	0x94, 0x32,	0xA6, 0xC2,	0x23, 0x3D,
	0xEE, 0x4C,	0x95, 0x0B,	0x42, 0xFA,	0xC3, 0x4E,
	0x08, 0x2E,	0xA1, 0x66,	0x28, 0xD9,	0x24, 0xB2,
	0x76, 0x5B,	0xA2, 0x49,	0x6D, 0x8B,	0xD1, 0x25,
	0x72, 0xF8,	0xF6, 0x64,	0x86, 0x68,	0x98, 0x16,
	0xD4, 0xA4,	0x5C, 0xCC,	0x5D, 0x65,	0xB6, 0x92,
	0x6C, 0x70,	0x48, 0x50,	0xFD, 0xED,	0xB9, 0xDA,
	0x5E, 0x15,	0x46, 0x57,	0xA7, 0x8D,	0x9D, 0x84,
	0x90, 0xD8,	0xAB, 0x00,	0x8C, 0xBC,	0xD3, 0x0A,
	0xF7, 0xE4,	0x58, 0x05,	0xB8, 0xB3,	0x45, 0x06,
	0xD0, 0x2C,	0x1E, 0x8F,	0xCA, 0x3F,	0x0F, 0x02,
	0xC1, 0xAF,	0xBD, 0x03,	0x01, 0x13,	0x8A, 0x6B,
	0x3A, 0x91,	0x11, 0x41,	0x4F, 0x67,	0xDC, 0xEA,
	0x97, 0xF2,	0xCF, 0xCE,	0xF0, 0xB4,	0xE6, 0x73,
	0x96, 0xAC,	0x74, 0x22,	0xE7, 0xAD,	0x35, 0x85,
	0xE2, 0xF9,	0x37, 0xE8,	0x1C, 0x75,	0xDF, 0x6E,
	0x47, 0xF1,	0x1A, 0x71,	0x1D, 0x29,	0xC5, 0x89,
	0x6F, 0xB7,	0x62, 0x0E,	0xAA, 0x18,	0xBE, 0x1B,
	0xFC, 0x56,	0x3E, 0x4B,	0xC6, 0xD2,	0x79, 0x20,
	0x9A, 0xDB,	0xC0, 0xFE,	0x78, 0xCD,	0x5A, 0xF4,
	0x1F, 0xDD,	0xA8, 0x33,	0x88, 0x07,	0xC7, 0x31,
	0xB1, 0x12,	0x10, 0x59,	0x27, 0x80,	0xEC, 0x5F,
	0x60, 0x51,	0x7F, 0xA9,	0x19, 0xB5,	0x4A, 0x0D,
	0x2D, 0xE5,	0x7A, 0x9F,	0x93, 0xC9,	0x9C, 0xEF,
	0xA0, 0xE0,	0x3B, 0x4D,	0xAE, 0x2A,	0xF5, 0xB0,
	0xC8, 0xEB,	0xBB, 0x3C,	0x83, 0x53,	0x99, 0x61,
	0x17, 0x2B,	0x04, 0x7E,	0xBA, 0x77,	0xD6, 0x26,
	0xE1, 0x69,	0x14, 0x63,	0x55, 0x21,	0x0C, 0x7D
};

/* reverse table */

#define	RT \
\
	V(51,F4,A7,50),	V(7E,41,65,53),	V(1A,17,A4,C3),	V(3A,27,5E,96),	\
	V(3B,AB,6B,CB),	V(1F,9D,45,F1),	V(AC,FA,58,AB),	V(4B,E3,03,93),	\
	V(20,30,FA,55),	V(AD,76,6D,F6),	V(88,CC,76,91),	V(F5,02,4C,25),	\
	V(4F,E5,D7,FC),	V(C5,2A,CB,D7),	V(26,35,44,80),	V(B5,62,A3,8F),	\
	V(DE,B1,5A,49),	V(25,BA,1B,67),	V(45,EA,0E,98),	V(5D,FE,C0,E1),	\
	V(C3,2F,75,02),	V(81,4C,F0,12),	V(8D,46,97,A3),	V(6B,D3,F9,C6),	\
	V(03,8F,5F,E7),	V(15,92,9C,95),	V(BF,6D,7A,EB),	V(95,52,59,DA),	\
	V(D4,BE,83,2D),	V(58,74,21,D3),	V(49,E0,69,29),	V(8E,C9,C8,44),	\
	V(75,C2,89,6A),	V(F4,8E,79,78),	V(99,58,3E,6B),	V(27,B9,71,DD),	\
	V(BE,E1,4F,B6),	V(F0,88,AD,17),	V(C9,20,AC,66),	V(7D,CE,3A,B4),	\
	V(63,DF,4A,18),	V(E5,1A,31,82),	V(97,51,33,60),	V(62,53,7F,45),	\
	V(B1,64,77,E0),	V(BB,6B,AE,84),	V(FE,81,A0,1C),	V(F9,08,2B,94),	\
	V(70,48,68,58),	V(8F,45,FD,19),	V(94,DE,6C,87),	V(52,7B,F8,B7),	\
	V(AB,73,D3,23),	V(72,4B,02,E2),	V(E3,1F,8F,57),	V(66,55,AB,2A),	\
	V(B2,EB,28,07),	V(2F,B5,C2,03),	V(86,C5,7B,9A),	V(D3,37,08,A5),	\
	V(30,28,87,F2),	V(23,BF,A5,B2),	V(02,03,6A,BA),	V(ED,16,82,5C),	\
	V(8A,CF,1C,2B),	V(A7,79,B4,92),	V(F3,07,F2,F0),	V(4E,69,E2,A1),	\
	V(65,DA,F4,CD),	V(06,05,BE,D5),	V(D1,34,62,1F),	V(C4,A6,FE,8A),	\
	V(34,2E,53,9D),	V(A2,F3,55,A0),	V(05,8A,E1,32),	V(A4,F6,EB,75),	\
	V(0B,83,EC,39),	V(40,60,EF,AA),	V(5E,71,9F,06),	V(BD,6E,10,51),	\
	V(3E,21,8A,F9),	V(96,DD,06,3D),	V(DD,3E,05,AE),	V(4D,E6,BD,46),	\
	V(91,54,8D,B5),	V(71,C4,5D,05),	V(04,06,D4,6F),	V(60,50,15,FF),	\
	V(19,98,FB,24),	V(D6,BD,E9,97),	V(89,40,43,CC),	V(67,D9,9E,77),	\
	V(B0,E8,42,BD),	V(07,89,8B,88),	V(E7,19,5B,38),	V(79,C8,EE,DB),	\
	V(A1,7C,0A,47),	V(7C,42,0F,E9),	V(F8,84,1E,C9),	V(00,00,00,00),	\
	V(09,80,86,83),	V(32,2B,ED,48),	V(1E,11,70,AC),	V(6C,5A,72,4E),	\
	V(FD,0E,FF,FB),	V(0F,85,38,56),	V(3D,AE,D5,1E),	V(36,2D,39,27),	\
	V(0A,0F,D9,64),	V(68,5C,A6,21),	V(9B,5B,54,D1),	V(24,36,2E,3A),	\
	V(0C,0A,67,B1),	V(93,57,E7,0F),	V(B4,EE,96,D2),	V(1B,9B,91,9E),	\
	V(80,C0,C5,4F),	V(61,DC,20,A2),	V(5A,77,4B,69),	V(1C,12,1A,16),	\
	V(E2,93,BA,0A),	V(C0,A0,2A,E5),	V(3C,22,E0,43),	V(12,1B,17,1D),	\
	V(0E,09,0D,0B),	V(F2,8B,C7,AD),	V(2D,B6,A8,B9),	V(14,1E,A9,C8),	\
	V(57,F1,19,85),	V(AF,75,07,4C),	V(EE,99,DD,BB),	V(A3,7F,60,FD),	\
	V(F7,01,26,9F),	V(5C,72,F5,BC),	V(44,66,3B,C5),	V(5B,FB,7E,34),	\
	V(8B,43,29,76),	V(CB,23,C6,DC),	V(B6,ED,FC,68),	V(B8,E4,F1,63),	\
	V(D7,31,DC,CA),	V(42,63,85,10),	V(13,97,22,40),	V(84,C6,11,20),	\
	V(85,4A,24,7D),	V(D2,BB,3D,F8),	V(AE,F9,32,11),	V(C7,29,A1,6D),	\
	V(1D,9E,2F,4B),	V(DC,B2,30,F3),	V(0D,86,52,EC),	V(77,C1,E3,D0),	\
	V(2B,B3,16,6C),	V(A9,70,B9,99),	V(11,94,48,FA),	V(47,E9,64,22),	\
	V(A8,FC,8C,C4),	V(A0,F0,3F,1A),	V(56,7D,2C,D8),	V(22,33,90,EF),	\
	V(87,49,4E,C7),	V(D9,38,D1,C1),	V(8C,CA,A2,FE),	V(98,D4,0B,36),	\
	V(A6,F5,81,CF),	V(A5,7A,DE,28),	V(DA,B7,8E,26),	V(3F,AD,BF,A4),	\
	V(2C,3A,9D,E4),	V(50,78,92,0D),	V(6A,5F,CC,9B),	V(54,7E,46,62),	\
	V(F6,8D,13,C2),	V(90,D8,B8,E8),	V(2E,39,F7,5E),	V(82,C3,AF,F5),	\
	V(9F,5D,80,BE),	V(69,D0,93,7C),	V(6F,D5,2D,A9),	V(CF,25,12,B3),	\
	V(C8,AC,99,3B),	V(10,18,7D,A7),	V(E8,9C,63,6E),	V(DB,3B,BB,7B),	\
	V(CD,26,78,09),	V(6E,59,18,F4),	V(EC,9A,B7,01),	V(83,4F,9A,A8),	\
	V(E6,95,6E,65),	V(AA,FF,E6,7E),	V(21,BC,CF,08),	V(EF,15,E8,E6),	\
	V(BA,E7,9B,D9),	V(4A,6F,36,CE),	V(EA,9F,09,D4),	V(29,B0,7C,D6),	\
	V(31,A4,B2,AF),	V(2A,3F,23,31),	V(C6,A5,94,30),	V(35,A2,66,C0),	\
	V(74,4E,BC,37),	V(FC,82,CA,A6),	V(E0,90,D0,B0),	V(33,A7,D8,15),	\
	V(F1,04,98,4A),	V(41,EC,DA,F7),	V(7F,CD,50,0E),	V(17,91,F6,2F),	\
	V(76,4D,D6,8D),	V(43,EF,B0,4D),	V(CC,AA,4D,54),	V(E4,96,04,DF),	\
	V(9E,D1,B5,E3),	V(4C,6A,88,1B),	V(C1,2C,1F,B8),	V(46,65,51,7F),	\
	V(9D,5E,EA,04),	V(01,8C,35,5D),	V(FA,87,74,73),	V(FB,0B,41,2E),	\
	V(B3,67,1D,5A),	V(92,DB,D2,52),	V(E9,10,56,33),	V(6D,D6,47,13),	\
	V(9A,D7,61,8C),	V(37,A1,0C,7A),	V(59,F8,14,8E),	V(EB,13,3C,89),	\
	V(CE,A9,27,EE),	V(B7,61,C9,35),	V(E1,1C,E5,ED),	V(7A,47,B1,3C),	\
	V(9C,D2,DF,59),	V(55,F2,73,3F),	V(18,14,CE,79),	V(73,C7,37,BF),	\
	V(53,F7,CD,EA),	V(5F,FD,AA,5B),	V(DF,3D,6F,14),	V(78,44,DB,86),	\
	V(CA,AF,F3,81),	V(B9,68,C4,3E),	V(38,24,34,2C),	V(C2,A3,40,5F),	\
	V(16,1D,C3,72),	V(BC,E2,25,0C),	V(28,3C,49,8B),	V(FF,0D,95,41),	\
	V(39,A8,01,71),	V(08,0C,B3,DE),	V(D8,B4,E4,9C),	V(64,56,C1,90),	\
	V(7B,CB,84,61),	V(D5,32,B6,70),	V(48,6C,5C,74),	V(D0,B8,57,42)

#define	V(a,b,c,d) 0x##a##b##c##d
static uint32 RT0[256] = { RT };
#undef V

#define	V(a,b,c,d) 0x##d##a##b##c
static uint32 RT1[256] = { RT };
#undef V

#define	V(a,b,c,d) 0x##c##d##a##b
static uint32 RT2[256] = { RT };
#undef V

#define	V(a,b,c,d) 0x##b##c##d##a
static uint32 RT3[256] = { RT };
#undef V

#undef RT

/* round constants */

static uint32 RCON[10] =
{
	0x01000000,	0x02000000,	0x04000000,	0x08000000,
	0x10000000,	0x20000000,	0x40000000,	0x80000000,
	0x1B000000,	0x36000000
};

/* key schedule	tables */

static int KT_init = 1;

static uint32 KT0[256];
static uint32 KT1[256];
static uint32 KT2[256];
static uint32 KT3[256];

/* platform-independant	32-bit integer manipulation	macros */

#define	GET_UINT32(n,b,i)						\
{												\
	(n)	= (	(uint32) (b)[(i)	] << 24	)		\
		| (	(uint32) (b)[(i) + 1] << 16	)		\
		| (	(uint32) (b)[(i) + 2] <<  8	)		\
		| (	(uint32) (b)[(i) + 3]		);		\
}

#define	PUT_UINT32(n,b,i)						\
{												\
	(b)[(i)	   ] = (uint8) ( (n) >>	24 );		\
	(b)[(i)	+ 1] = (uint8) ( (n) >>	16 );		\
	(b)[(i)	+ 2] = (uint8) ( (n) >>	 8 );		\
	(b)[(i)	+ 3] = (uint8) ( (n)	   );		\
}

/* AES key scheduling routine */

int	rtmp_aes_set_key( aes_context *ctx, uint8 *key, int nbits )
{
	int	i;
	uint32 *RK,	*SK;

	switch(	nbits )
	{
		case 128: ctx->nr =	10;	break;
		case 192: ctx->nr =	12;	break;
		case 256: ctx->nr =	14;	break;
		default	: return( 1	);
	}

	RK = ctx->erk;

	for( i = 0;	i <	(nbits >> 5); i++ )
	{
		GET_UINT32(	RK[i], key,	i *	4 );
	}

	/* setup encryption	round keys */

	switch(	nbits )
	{
	case 128:

		for( i = 0;	i <	10;	i++, RK	+= 4 )
		{
			RK[4]  = RK[0] ^ RCON[i] ^
						( FSb[ (uint8) ( RK[3] >> 16 ) ] <<	24 ) ^
						( FSb[ (uint8) ( RK[3] >>  8 ) ] <<	16 ) ^
						( FSb[ (uint8) ( RK[3]		 ) ] <<	 8 ) ^
						( FSb[ (uint8) ( RK[3] >> 24 ) ]	   );

			RK[5]  = RK[1] ^ RK[4];
			RK[6]  = RK[2] ^ RK[5];
			RK[7]  = RK[3] ^ RK[6];
		}
		break;

	case 192:

		for( i = 0;	i <	8; i++,	RK += 6	)
		{
			RK[6]  = RK[0] ^ RCON[i] ^
						( FSb[ (uint8) ( RK[5] >> 16 ) ] <<	24 ) ^
						( FSb[ (uint8) ( RK[5] >>  8 ) ] <<	16 ) ^
						( FSb[ (uint8) ( RK[5]		 ) ] <<	 8 ) ^
						( FSb[ (uint8) ( RK[5] >> 24 ) ]	   );

			RK[7]  = RK[1] ^ RK[6];
			RK[8]  = RK[2] ^ RK[7];
			RK[9]  = RK[3] ^ RK[8];
			RK[10] = RK[4] ^ RK[9];
			RK[11] = RK[5] ^ RK[10];
		}
		break;

	case 256:

		for( i = 0;	i <	7; i++,	RK += 8	)
		{
			RK[8]  = RK[0] ^ RCON[i] ^
						( FSb[ (uint8) ( RK[7] >> 16 ) ] <<	24 ) ^
						( FSb[ (uint8) ( RK[7] >>  8 ) ] <<	16 ) ^
						( FSb[ (uint8) ( RK[7]		 ) ] <<	 8 ) ^
						( FSb[ (uint8) ( RK[7] >> 24 ) ]	   );

			RK[9]  = RK[1] ^ RK[8];
			RK[10] = RK[2] ^ RK[9];
			RK[11] = RK[3] ^ RK[10];

			RK[12] = RK[4] ^
						( FSb[ (uint8) ( RK[11]	>> 24 )	] << 24	) ^
						( FSb[ (uint8) ( RK[11]	>> 16 )	] << 16	) ^
						( FSb[ (uint8) ( RK[11]	>>	8 )	] <<  8	) ^
						( FSb[ (uint8) ( RK[11]		  )	]		);

			RK[13] = RK[5] ^ RK[12];
			RK[14] = RK[6] ^ RK[13];
			RK[15] = RK[7] ^ RK[14];
		}
		break;
	}

	/* setup decryption	round keys */

	if(	KT_init	)
	{
		for( i = 0;	i <	256; i++ )
		{
			KT0[i] = RT0[ FSb[i] ];
			KT1[i] = RT1[ FSb[i] ];
			KT2[i] = RT2[ FSb[i] ];
			KT3[i] = RT3[ FSb[i] ];
		}

		KT_init	= 0;
	}

	SK = ctx->drk;

	*SK++ =	*RK++;
	*SK++ =	*RK++;
	*SK++ =	*RK++;
	*SK++ =	*RK++;

	for( i = 1;	i <	ctx->nr; i++ )
	{
		RK -= 8;

		*SK++ =	KT0[ (uint8) ( *RK >> 24 ) ] ^
				KT1[ (uint8) ( *RK >> 16 ) ] ^
				KT2[ (uint8) ( *RK >>  8 ) ] ^
				KT3[ (uint8) ( *RK		 ) ]; RK++;

		*SK++ =	KT0[ (uint8) ( *RK >> 24 ) ] ^
				KT1[ (uint8) ( *RK >> 16 ) ] ^
				KT2[ (uint8) ( *RK >>  8 ) ] ^
				KT3[ (uint8) ( *RK		 ) ]; RK++;

		*SK++ =	KT0[ (uint8) ( *RK >> 24 ) ] ^
				KT1[ (uint8) ( *RK >> 16 ) ] ^
				KT2[ (uint8) ( *RK >>  8 ) ] ^
				KT3[ (uint8) ( *RK		 ) ]; RK++;

		*SK++ =	KT0[ (uint8) ( *RK >> 24 ) ] ^
				KT1[ (uint8) ( *RK >> 16 ) ] ^
				KT2[ (uint8) ( *RK >>  8 ) ] ^
				KT3[ (uint8) ( *RK		 ) ]; RK++;
	}

	RK -= 8;

	*SK++ =	*RK++;
	*SK++ =	*RK++;
	*SK++ =	*RK++;
	*SK++ =	*RK++;

	return(	0 );
}

/* AES 128-bit block encryption	routine	*/

void rtmp_aes_encrypt(aes_context *ctx, uint8 input[16],	uint8 output[16] )
{
	uint32 *RK,	X0,	X1,	X2,	X3,	Y0,	Y1,	Y2,	Y3;

	RK = ctx->erk;
	GET_UINT32(	X0,	input,	0 ); X0	^= RK[0];
	GET_UINT32(	X1,	input,	4 ); X1	^= RK[1];
	GET_UINT32(	X2,	input,	8 ); X2	^= RK[2];
	GET_UINT32(	X3,	input, 12 ); X3	^= RK[3];

#define	AES_FROUND(X0,X1,X2,X3,Y0,Y1,Y2,Y3)		\
{												\
	RK += 4;									\
												\
	X0 = RK[0] ^ FT0[ (uint8) (	Y0 >> 24 ) ] ^	\
				 FT1[ (uint8) (	Y1 >> 16 ) ] ^	\
				 FT2[ (uint8) (	Y2 >>  8 ) ] ^	\
				 FT3[ (uint8) (	Y3		 ) ];	\
												\
	X1 = RK[1] ^ FT0[ (uint8) (	Y1 >> 24 ) ] ^	\
				 FT1[ (uint8) (	Y2 >> 16 ) ] ^	\
				 FT2[ (uint8) (	Y3 >>  8 ) ] ^	\
				 FT3[ (uint8) (	Y0		 ) ];	\
												\
	X2 = RK[2] ^ FT0[ (uint8) (	Y2 >> 24 ) ] ^	\
				 FT1[ (uint8) (	Y3 >> 16 ) ] ^	\
				 FT2[ (uint8) (	Y0 >>  8 ) ] ^	\
				 FT3[ (uint8) (	Y1		 ) ];	\
												\
	X3 = RK[3] ^ FT0[ (uint8) (	Y3 >> 24 ) ] ^	\
				 FT1[ (uint8) (	Y0 >> 16 ) ] ^	\
				 FT2[ (uint8) (	Y1 >>  8 ) ] ^	\
				 FT3[ (uint8) (	Y2		 ) ];	\
}

	AES_FROUND(	Y0,	Y1,	Y2,	Y3,	X0,	X1,	X2,	X3 );		/* round 1 */
	AES_FROUND(	X0,	X1,	X2,	X3,	Y0,	Y1,	Y2,	Y3 );		/* round 2 */
	AES_FROUND(	Y0,	Y1,	Y2,	Y3,	X0,	X1,	X2,	X3 );		/* round 3 */
	AES_FROUND(	X0,	X1,	X2,	X3,	Y0,	Y1,	Y2,	Y3 );		/* round 4 */
	AES_FROUND(	Y0,	Y1,	Y2,	Y3,	X0,	X1,	X2,	X3 );		/* round 5 */
	AES_FROUND(	X0,	X1,	X2,	X3,	Y0,	Y1,	Y2,	Y3 );		/* round 6 */
	AES_FROUND(	Y0,	Y1,	Y2,	Y3,	X0,	X1,	X2,	X3 );		/* round 7 */
	AES_FROUND(	X0,	X1,	X2,	X3,	Y0,	Y1,	Y2,	Y3 );		/* round 8 */
	AES_FROUND(	Y0,	Y1,	Y2,	Y3,	X0,	X1,	X2,	X3 );		/* round 9 */

	if(	ctx->nr	> 10 )
	{
		AES_FROUND(	X0,	X1,	X2,	X3,	Y0,	Y1,	Y2,	Y3 );	/* round 10	*/
		AES_FROUND(	Y0,	Y1,	Y2,	Y3,	X0,	X1,	X2,	X3 );	/* round 11	*/
	}

	if(	ctx->nr	> 12 )
	{
		AES_FROUND(	X0,	X1,	X2,	X3,	Y0,	Y1,	Y2,	Y3 );	/* round 12	*/
		AES_FROUND(	Y0,	Y1,	Y2,	Y3,	X0,	X1,	X2,	X3 );	/* round 13	*/
	}

	/* last	round */

	RK += 4;

	X0 = RK[0] ^ ( FSb[	(uint8)	( Y0 >>	24 ) ] << 24 ) ^
				 ( FSb[	(uint8)	( Y1 >>	16 ) ] << 16 ) ^
				 ( FSb[	(uint8)	( Y2 >>	 8 ) ] <<  8 ) ^
				 ( FSb[	(uint8)	( Y3	   ) ]		 );

	X1 = RK[1] ^ ( FSb[	(uint8)	( Y1 >>	24 ) ] << 24 ) ^
				 ( FSb[	(uint8)	( Y2 >>	16 ) ] << 16 ) ^
				 ( FSb[	(uint8)	( Y3 >>	 8 ) ] <<  8 ) ^
				 ( FSb[	(uint8)	( Y0	   ) ]		 );

	X2 = RK[2] ^ ( FSb[	(uint8)	( Y2 >>	24 ) ] << 24 ) ^
				 ( FSb[	(uint8)	( Y3 >>	16 ) ] << 16 ) ^
				 ( FSb[	(uint8)	( Y0 >>	 8 ) ] <<  8 ) ^
				 ( FSb[	(uint8)	( Y1	   ) ]		 );

	X3 = RK[3] ^ ( FSb[	(uint8)	( Y3 >>	24 ) ] << 24 ) ^
				 ( FSb[	(uint8)	( Y0 >>	16 ) ] << 16 ) ^
				 ( FSb[	(uint8)	( Y1 >>	 8 ) ] <<  8 ) ^
				 ( FSb[	(uint8)	( Y2	   ) ]		 );

	PUT_UINT32(	X0,	output,	 0 );
	PUT_UINT32(	X1,	output,	 4 );
	PUT_UINT32(	X2,	output,	 8 );
	PUT_UINT32(	X3,	output,	12 );
}

/* AES 128-bit block decryption	routine	*/

void rtmp_aes_decrypt( aes_context *ctx,	uint8 input[16], uint8 output[16] )
{
	uint32 *RK,	X0,	X1,	X2,	X3,	Y0,	Y1,	Y2,	Y3;

	RK = ctx->drk;

	GET_UINT32(	X0,	input,	0 ); X0	^= RK[0];
	GET_UINT32(	X1,	input,	4 ); X1	^= RK[1];
	GET_UINT32(	X2,	input,	8 ); X2	^= RK[2];
	GET_UINT32(	X3,	input, 12 ); X3	^= RK[3];

#define	AES_RROUND(X0,X1,X2,X3,Y0,Y1,Y2,Y3)		\
{												\
	RK += 4;									\
												\
	X0 = RK[0] ^ RT0[ (uint8) (	Y0 >> 24 ) ] ^	\
				 RT1[ (uint8) (	Y3 >> 16 ) ] ^	\
				 RT2[ (uint8) (	Y2 >>  8 ) ] ^	\
				 RT3[ (uint8) (	Y1		 ) ];	\
												\
	X1 = RK[1] ^ RT0[ (uint8) (	Y1 >> 24 ) ] ^	\
				 RT1[ (uint8) (	Y0 >> 16 ) ] ^	\
				 RT2[ (uint8) (	Y3 >>  8 ) ] ^	\
				 RT3[ (uint8) (	Y2		 ) ];	\
												\
	X2 = RK[2] ^ RT0[ (uint8) (	Y2 >> 24 ) ] ^	\
				 RT1[ (uint8) (	Y1 >> 16 ) ] ^	\
				 RT2[ (uint8) (	Y0 >>  8 ) ] ^	\
				 RT3[ (uint8) (	Y3		 ) ];	\
												\
	X3 = RK[3] ^ RT0[ (uint8) (	Y3 >> 24 ) ] ^	\
				 RT1[ (uint8) (	Y2 >> 16 ) ] ^	\
				 RT2[ (uint8) (	Y1 >>  8 ) ] ^	\
				 RT3[ (uint8) (	Y0		 ) ];	\
}

	AES_RROUND(	Y0,	Y1,	Y2,	Y3,	X0,	X1,	X2,	X3 );		/* round 1 */
	AES_RROUND(	X0,	X1,	X2,	X3,	Y0,	Y1,	Y2,	Y3 );		/* round 2 */
	AES_RROUND(	Y0,	Y1,	Y2,	Y3,	X0,	X1,	X2,	X3 );		/* round 3 */
	AES_RROUND(	X0,	X1,	X2,	X3,	Y0,	Y1,	Y2,	Y3 );		/* round 4 */
	AES_RROUND(	Y0,	Y1,	Y2,	Y3,	X0,	X1,	X2,	X3 );		/* round 5 */
	AES_RROUND(	X0,	X1,	X2,	X3,	Y0,	Y1,	Y2,	Y3 );		/* round 6 */
	AES_RROUND(	Y0,	Y1,	Y2,	Y3,	X0,	X1,	X2,	X3 );		/* round 7 */
	AES_RROUND(	X0,	X1,	X2,	X3,	Y0,	Y1,	Y2,	Y3 );		/* round 8 */
	AES_RROUND(	Y0,	Y1,	Y2,	Y3,	X0,	X1,	X2,	X3 );		/* round 9 */

	if(	ctx->nr	> 10 )
	{
		AES_RROUND(	X0,	X1,	X2,	X3,	Y0,	Y1,	Y2,	Y3 );	/* round 10	*/
		AES_RROUND(	Y0,	Y1,	Y2,	Y3,	X0,	X1,	X2,	X3 );	/* round 11	*/
	}

	if(	ctx->nr	> 12 )
	{
		AES_RROUND(	X0,	X1,	X2,	X3,	Y0,	Y1,	Y2,	Y3 );	/* round 12	*/
		AES_RROUND(	Y0,	Y1,	Y2,	Y3,	X0,	X1,	X2,	X3 );	/* round 13	*/
	}

	/* last	round */

	RK += 4;

	X0 = RK[0] ^ ( RSb[	(uint8)	( Y0 >>	24 ) ] << 24 ) ^
				 ( RSb[	(uint8)	( Y3 >>	16 ) ] << 16 ) ^
				 ( RSb[	(uint8)	( Y2 >>	 8 ) ] <<  8 ) ^
				 ( RSb[	(uint8)	( Y1	   ) ]		 );

	X1 = RK[1] ^ ( RSb[	(uint8)	( Y1 >>	24 ) ] << 24 ) ^
				 ( RSb[	(uint8)	( Y0 >>	16 ) ] << 16 ) ^
				 ( RSb[	(uint8)	( Y3 >>	 8 ) ] <<  8 ) ^
				 ( RSb[	(uint8)	( Y2	   ) ]		 );

	X2 = RK[2] ^ ( RSb[	(uint8)	( Y2 >>	24 ) ] << 24 ) ^
				 ( RSb[	(uint8)	( Y1 >>	16 ) ] << 16 ) ^
				 ( RSb[	(uint8)	( Y0 >>	 8 ) ] <<  8 ) ^
				 ( RSb[	(uint8)	( Y3	   ) ]		 );

	X3 = RK[3] ^ ( RSb[	(uint8)	( Y3 >>	24 ) ] << 24 ) ^
				 ( RSb[	(uint8)	( Y2 >>	16 ) ] << 16 ) ^
				 ( RSb[	(uint8)	( Y1 >>	 8 ) ] <<  8 ) ^
				 ( RSb[	(uint8)	( Y0	   ) ]		 );

	PUT_UINT32(	X0,	output,	 0 );
	PUT_UINT32(	X1,	output,	 4 );
	PUT_UINT32(	X2,	output,	 8 );
	PUT_UINT32(	X3,	output,	12 );
}

/*
    ==========================================================================
    Description:
        ENCRYPT AES GTK before sending in EAPOL frame.
        AES GTK length = 128 bit,  so fix blocks for aes-key-wrap as 2 in this function.
        This function references to RFC 3394 for aes key wrap algorithm.
    Return:
    ==========================================================================
*/  
VOID AES_GTK_KEY_WRAP( 
    IN UCHAR    *key,
    IN UCHAR    *plaintext,
    IN UINT     p_len,
    OUT UCHAR   *ciphertext,
	OUT UINT    *c_len) 
{
    UCHAR       A[8], BIN[16], BOUT[16];
/*    UCHAR       R[512];
*/
	UCHAR		*R = NULL;
    INT         num_blocks = p_len/8;   /* unit:64bits
*/
    INT         i, j;
/*    aes_context aesctx;
*/
	aes_context *paesctx = NULL;
    UCHAR       xor;


	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&R, 512);
	if (R == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		goto LabelErr;
	}
	os_alloc_mem(NULL, (UCHAR **)&paesctx, sizeof(aes_context));
	if (paesctx == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		goto LabelErr;
	}

    rtmp_aes_set_key(paesctx, key, 128);

    /* Init IA
*/
    for (i = 0; i < 8; i++)
        A[i] = 0xa6;

    /*Input plaintext
*/
    for (i = 0; i < num_blocks; i++)
    {
        for (j = 0 ; j < 8; j++)
            R[8 * (i + 1) + j] = plaintext[8 * i + j];
    }

    /* Key Mix
*/
    for (j = 0; j < 6; j++)
    {
        for(i = 1; i <= num_blocks; i++)
        {
            /*phase 1
*/
            NdisMoveMemory(BIN, A, 8);
            NdisMoveMemory(&BIN[8], &R[8 * i], 8);
            rtmp_aes_encrypt(paesctx, BIN, BOUT);

            NdisMoveMemory(A, &BOUT[0], 8);
            xor = num_blocks * j + i;
            A[7] = BOUT[7] ^ xor;
            NdisMoveMemory(&R[8 * i], &BOUT[8], 8);
        }
    }

    /* Output ciphertext
*/
    NdisMoveMemory(ciphertext, A, 8);

    for (i = 1; i <= num_blocks; i++)
    {
        for (j = 0 ; j < 8; j++)
            ciphertext[8 * i + j] = R[8 * i + j];
    }
    *c_len = p_len + 8;

LabelErr:
	if (R != NULL)
		os_free_mem(NULL, R);
	if (paesctx != NULL)
		os_free_mem(NULL, paesctx);
}


/*
	========================================================================
	
	Routine Description:
		Misc function to decrypt AES body
	
	Arguments:
			
	Return Value:
	
	Note:
		This function references to	RFC	3394 for aes key unwrap algorithm.
			
	========================================================================
*/
VOID	AES_GTK_KEY_UNWRAP( 
	IN	UCHAR	*key,
	OUT	UCHAR	*plaintext,
	OUT	UINT    *p_len,
	IN	UCHAR	*ciphertext,
	IN	UINT    c_len)
	
{
	UCHAR       A[8], BIN[16], BOUT[16];
	UCHAR       xor;
	INT         i, j;
/*	aes_context aesctx;
*/
	aes_context *paesctx = NULL;
	UCHAR       *R = NULL;
	INT         num_blocks = c_len/8;	/* unit:64bits
*/

	
	os_alloc_mem(NULL, (PUCHAR *)&R, 512);
	if (R == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("!!!AES_GTK_KEY_UNWRAP: no memory!!!\n"));
        return;
    } /* End of if */

	os_alloc_mem(NULL, (UCHAR **)&paesctx, sizeof(aes_context));
	if (paesctx == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("!!!AES_GTK_KEY_UNWRAP: no memory!!!\n"));
		goto LabelOK;
	}

	/* Initialize
*/
	NdisMoveMemory(A, ciphertext, 8);
	/*Input plaintext
*/
	for(i = 0; i < (c_len-8); i++)
	{
		R[ i] = ciphertext[i + 8];
	}

	rtmp_aes_set_key(paesctx, key, 128);

	for(j = 5; j >= 0; j--)
	{
		for(i = (num_blocks-1); i > 0; i--)
		{
			xor = (num_blocks -1 )* j + i;
			NdisMoveMemory(BIN, A, 8);
			BIN[7] = A[7] ^ xor;
			NdisMoveMemory(&BIN[8], &R[(i-1)*8], 8);
			rtmp_aes_decrypt(paesctx, BIN, BOUT);
			NdisMoveMemory(A, &BOUT[0], 8);
			NdisMoveMemory(&R[(i-1)*8], &BOUT[8], 8);
		}
	}

	/* OUTPUT
*/
	for(i = 0; i < c_len; i++)
	{
		plaintext[i] = R[i];
	}

	DBGPRINT_RAW(RT_DEBUG_INFO, ("plaintext = \n"));
	for(i = 0; i < (num_blocks *8); i++)
	{
		DBGPRINT_RAW(RT_DEBUG_INFO, ("%2x ", plaintext[i]));
		if(i%16 == 15)
			DBGPRINT_RAW(RT_DEBUG_INFO, ("\n "));
	}
	DBGPRINT_RAW(RT_DEBUG_INFO, ("\n  \n"));	

    *p_len = c_len - 8;    

LabelOK:
	if (R != NULL)
		os_free_mem(NULL, R);

	if (paesctx != NULL)
		os_free_mem(NULL, paesctx);
}


/* The value given by [x^(i-1),{00},{00},{00}], with x^(i-1) being powers of x in the field GF(2^8). */
static const UINT32 aes_rcon[] = {
	0x00000000, 0x01000000, 0x02000000, 0x04000000, 
    0x08000000, 0x10000000, 0x20000000, 0x40000000, 
    0x80000000, 0x1B000000, 0x36000000};

static const UINT8 aes_sbox_enc[] = {
  /*  0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f    */
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7 ,0xab, 0x76, /* 0 */
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4 ,0x72, 0xc0, /* 1 */
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8 ,0x31, 0x15, /* 2 */
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27 ,0xb2, 0x75, /* 3 */
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3 ,0x2f, 0x84, /* 4 */
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c ,0x58, 0xcf, /* 5 */
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c ,0x9f, 0xa8, /* 6 */
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff ,0xf3, 0xd2, /* 7 */
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d ,0x19, 0x73, /* 8 */
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e ,0x0b, 0xdb, /* 9 */
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95 ,0xe4, 0x79, /* a */
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a ,0xae, 0x08, /* b */
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd ,0x8b, 0x8a, /* c */
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1 ,0x1d, 0x9e, /* d */
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55 ,0x28, 0xdf, /* e */
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54 ,0xbb, 0x16, /* f */
};

static const UINT8 aes_sbox_dec[] = {
  /*  0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f    */
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb, /* 0 */
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb, /* 1 */
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e, /* 2 */
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25, /* 3 */
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92, /* 4 */
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84, /* 5 */
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06, /* 6 */
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b, /* 7 */
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73, /* 8 */
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e, /* 9 */
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b, /* a */
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4, /* b */
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f, /* c */
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef, /* d */
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61, /* e */
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d, /* f */
};

/* ArrayIndex*{02} */
static const UINT8 aes_mul_2[] = {
  /*  0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f    */
    0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e, /* 0 */
    0x20, 0x22, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3a, 0x3c, 0x3e, /* 1 */
    0x40, 0x42, 0x44, 0x46, 0x48, 0x4a, 0x4c, 0x4e, 0x50, 0x52, 0x54, 0x56, 0x58, 0x5a, 0x5c, 0x5e, /* 2 */
    0x60, 0x62, 0x64, 0x66, 0x68, 0x6a, 0x6c, 0x6e, 0x70, 0x72, 0x74, 0x76, 0x78, 0x7a, 0x7c, 0x7e, /* 3 */
    0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c, 0x8e, 0x90, 0x92, 0x94, 0x96, 0x98, 0x9a, 0x9c, 0x9e, /* 4 */
    0xa0, 0xa2, 0xa4, 0xa6, 0xa8, 0xaa, 0xac, 0xae, 0xb0, 0xb2, 0xb4, 0xb6, 0xb8, 0xba, 0xbc, 0xbe, /* 5 */
    0xc0, 0xc2, 0xc4, 0xc6, 0xc8, 0xca, 0xcc, 0xce, 0xd0, 0xd2, 0xd4, 0xd6, 0xd8, 0xda, 0xdc, 0xde, /* 6 */
    0xe0, 0xe2, 0xe4, 0xe6, 0xe8, 0xea, 0xec, 0xee, 0xf0, 0xf2, 0xf4, 0xf6, 0xf8, 0xfa, 0xfc, 0xfe, /* 7 */
    0x1b, 0x19, 0x1f, 0x1d, 0x13, 0x11, 0x17, 0x15, 0x0b, 0x09, 0x0f, 0x0d, 0x03, 0x01, 0x07, 0x05, /* 8 */
    0x3b, 0x39, 0x3f, 0x3d, 0x33, 0x31, 0x37, 0x35, 0x2b, 0x29, 0x2f, 0x2d, 0x23, 0x21, 0x27, 0x25, /* 9 */
    0x5b, 0x59, 0x5f, 0x5d, 0x53, 0x51, 0x57, 0x55, 0x4b, 0x49, 0x4f, 0x4d, 0x43, 0x41, 0x47, 0x45, /* a */
    0x7b, 0x79, 0x7f, 0x7d, 0x73, 0x71, 0x77, 0x75, 0x6b, 0x69, 0x6f, 0x6d, 0x63, 0x61, 0x67, 0x65, /* b */
    0x9b, 0x99, 0x9f, 0x9d, 0x93, 0x91, 0x97, 0x95, 0x8b, 0x89, 0x8f, 0x8d, 0x83, 0x81, 0x87, 0x85, /* c */
    0xbb, 0xb9, 0xbf, 0xbd, 0xb3, 0xb1, 0xb7, 0xb5, 0xab, 0xa9, 0xaf, 0xad, 0xa3, 0xa1, 0xa7, 0xa5, /* d */
    0xdb, 0xd9, 0xdf, 0xdd, 0xd3, 0xd1, 0xd7, 0xd5, 0xcb, 0xc9, 0xcf, 0xcd, 0xc3, 0xc1, 0xc7, 0xc5, /* e */
    0xfb, 0xf9, 0xff, 0xfd, 0xf3, 0xf1, 0xf7, 0xf5, 0xeb, 0xe9, 0xef, 0xed, 0xe3, 0xe1, 0xe7, 0xe5, /* f */
};

/* ArrayIndex*{03} */
static const UINT8 aes_mul_3[] = {
  /*  0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f    */
    0x00, 0x03, 0x06, 0x05, 0x0c, 0x0f, 0x0a, 0x09, 0x18, 0x1b, 0x1e, 0x1d, 0x14, 0x17, 0x12, 0x11, /* 0 */
    0x30, 0x33, 0x36, 0x35, 0x3c, 0x3f, 0x3a, 0x39, 0x28, 0x2b, 0x2e, 0x2d, 0x24, 0x27, 0x22, 0x21, /* 1 */
    0x60, 0x63, 0x66, 0x65, 0x6c, 0x6f, 0x6a, 0x69, 0x78, 0x7b, 0x7e, 0x7d, 0x74, 0x77, 0x72, 0x71, /* 2 */
    0x50, 0x53, 0x56, 0x55, 0x5c, 0x5f, 0x5a, 0x59, 0x48, 0x4b, 0x4e, 0x4d, 0x44, 0x47, 0x42, 0x41, /* 3 */
    0xc0, 0xc3, 0xc6, 0xc5, 0xcc, 0xcf, 0xca, 0xc9, 0xd8, 0xdb, 0xde, 0xdd, 0xd4, 0xd7, 0xd2, 0xd1, /* 4 */
    0xf0, 0xf3, 0xf6, 0xf5, 0xfc, 0xff, 0xfa, 0xf9, 0xe8, 0xeb, 0xee, 0xed, 0xe4, 0xe7, 0xe2, 0xe1, /* 5 */
    0xa0, 0xa3, 0xa6, 0xa5, 0xac, 0xaf, 0xaa, 0xa9, 0xb8, 0xbb, 0xbe, 0xbd, 0xb4, 0xb7, 0xb2, 0xb1, /* 6 */
    0x90, 0x93, 0x96, 0x95, 0x9c, 0x9f, 0x9a, 0x99, 0x88, 0x8b, 0x8e, 0x8d, 0x84, 0x87, 0x82, 0x81, /* 7 */
    0x9b, 0x98, 0x9d, 0x9e, 0x97, 0x94, 0x91, 0x92, 0x83, 0x80, 0x85, 0x86, 0x8f, 0x8c, 0x89, 0x8a, /* 8 */
    0xab, 0xa8, 0xad, 0xae, 0xa7, 0xa4, 0xa1, 0xa2, 0xb3, 0xb0, 0xb5, 0xb6, 0xbf, 0xbc, 0xb9, 0xba, /* 9 */
    0xfb, 0xf8, 0xfd, 0xfe, 0xf7, 0xf4, 0xf1, 0xf2, 0xe3, 0xe0, 0xe5, 0xe6, 0xef, 0xec, 0xe9, 0xea, /* a */
    0xcb, 0xc8, 0xcd, 0xce, 0xc7, 0xc4, 0xc1, 0xc2, 0xd3, 0xd0, 0xd5, 0xd6, 0xdf, 0xdc, 0xd9, 0xda, /* b */
    0x5b, 0x58, 0x5d, 0x5e, 0x57, 0x54, 0x51, 0x52, 0x43, 0x40, 0x45, 0x46, 0x4f, 0x4c, 0x49, 0x4a, /* c */
    0x6b, 0x68, 0x6d, 0x6e, 0x67, 0x64, 0x61, 0x62, 0x73, 0x70, 0x75, 0x76, 0x7f, 0x7c, 0x79, 0x7a, /* d */
    0x3b, 0x38, 0x3d, 0x3e, 0x37, 0x34, 0x31, 0x32, 0x23, 0x20, 0x25, 0x26, 0x2f, 0x2c, 0x29, 0x2a, /* e */
    0x0b, 0x08, 0x0d, 0x0e, 0x07, 0x04, 0x01, 0x02, 0x13, 0x10, 0x15, 0x16, 0x1f, 0x1c, 0x19, 0x1a, /* f */
};

/* ArrayIndex*{09} */
static const UINT8 aes_mul_9[] = {
  /*  0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f    */
    0x00, 0x09, 0x12, 0x1b, 0x24, 0x2d, 0x36, 0x3f, 0x48, 0x41, 0x5a, 0x53, 0x6c, 0x65, 0x7e, 0x77, /* 0 */
    0x90, 0x99, 0x82, 0x8b, 0xb4, 0xbd, 0xa6, 0xaf, 0xd8, 0xd1, 0xca, 0xc3, 0xfc, 0xf5, 0xee, 0xe7, /* 1 */
    0x3b, 0x32, 0x29, 0x20, 0x1f, 0x16, 0x0d, 0x04, 0x73, 0x7a, 0x61, 0x68, 0x57, 0x5e, 0x45, 0x4c, /* 2 */
    0xab, 0xa2, 0xb9, 0xb0, 0x8f, 0x86, 0x9d, 0x94, 0xe3, 0xea, 0xf1, 0xf8, 0xc7, 0xce, 0xd5, 0xdc, /* 3 */
    0x76, 0x7f, 0x64, 0x6d, 0x52, 0x5b, 0x40, 0x49, 0x3e, 0x37, 0x2c, 0x25, 0x1a, 0x13, 0x08, 0x01, /* 4 */
    0xe6, 0xef, 0xf4, 0xfd, 0xc2, 0xcb, 0xd0, 0xd9, 0xae, 0xa7, 0xbc, 0xb5, 0x8a, 0x83, 0x98, 0x91, /* 5 */
    0x4d, 0x44, 0x5f, 0x56, 0x69, 0x60, 0x7b, 0x72, 0x05, 0x0c, 0x17, 0x1e, 0x21, 0x28, 0x33, 0x3a, /* 6 */
    0xdd, 0xd4, 0xcf, 0xc6, 0xf9, 0xf0, 0xeb, 0xe2, 0x95, 0x9c, 0x87, 0x8e, 0xb1, 0xb8, 0xa3, 0xaa, /* 7 */
    0xec, 0xe5, 0xfe, 0xf7, 0xc8, 0xc1, 0xda, 0xd3, 0xa4, 0xad, 0xb6, 0xbf, 0x80, 0x89, 0x92, 0x9b, /* 8 */
    0x7c, 0x75, 0x6e, 0x67, 0x58, 0x51, 0x4a, 0x43, 0x34, 0x3d, 0x26, 0x2f, 0x10, 0x19, 0x02, 0x0b, /* 9 */
    0xd7, 0xde, 0xc5, 0xcc, 0xf3, 0xfa, 0xe1, 0xe8, 0x9f, 0x96, 0x8d, 0x84, 0xbb, 0xb2, 0xa9, 0xa0, /* a */
    0x47, 0x4e, 0x55, 0x5c, 0x63, 0x6a, 0x71, 0x78, 0x0f, 0x06, 0x1d, 0x14, 0x2b, 0x22, 0x39, 0x30, /* b */
    0x9a, 0x93, 0x88, 0x81, 0xbe, 0xb7, 0xac, 0xa5, 0xd2, 0xdb, 0xc0, 0xc9, 0xf6, 0xff, 0xe4, 0xed, /* c */
    0x0a, 0x03, 0x18, 0x11, 0x2e, 0x27, 0x3c, 0x35, 0x42, 0x4b, 0x50, 0x59, 0x66, 0x6f, 0x74, 0x7d, /* d */
    0xa1, 0xa8, 0xb3, 0xba, 0x85, 0x8c, 0x97, 0x9e, 0xe9, 0xe0, 0xfb, 0xf2, 0xcd, 0xc4, 0xdf, 0xd6, /* e */
    0x31, 0x38, 0x23, 0x2a, 0x15, 0x1c, 0x07, 0x0e, 0x79, 0x70, 0x6b, 0x62, 0x5d, 0x54, 0x4f, 0x46, /* f */
};

/* ArrayIndex*{0b} */
static const UINT8 aes_mul_b[] = {
  /*  0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f    */
    0x00, 0x0b, 0x16, 0x1d, 0x2c, 0x27, 0x3a, 0x31, 0x58, 0x53, 0x4e, 0x45, 0x74, 0x7f, 0x62, 0x69, /* 0 */
    0xb0, 0xbb, 0xa6, 0xad, 0x9c, 0x97, 0x8a, 0x81, 0xe8, 0xe3, 0xfe, 0xf5, 0xc4, 0xcf, 0xd2, 0xd9, /* 1 */
    0x7b, 0x70, 0x6d, 0x66, 0x57, 0x5c, 0x41, 0x4a, 0x23, 0x28, 0x35, 0x3e, 0x0f, 0x04, 0x19, 0x12, /* 2 */
    0xcb, 0xc0, 0xdd, 0xd6, 0xe7, 0xec, 0xf1, 0xfa, 0x93, 0x98, 0x85, 0x8e, 0xbf, 0xb4, 0xa9, 0xa2, /* 3 */
    0xf6, 0xfd, 0xe0, 0xeb, 0xda, 0xd1, 0xcc, 0xc7, 0xae, 0xa5, 0xb8, 0xb3, 0x82, 0x89, 0x94, 0x9f, /* 4 */
    0x46, 0x4d, 0x50, 0x5b, 0x6a, 0x61, 0x7c, 0x77, 0x1e, 0x15, 0x08, 0x03, 0x32, 0x39, 0x24, 0x2f, /* 5 */
    0x8d, 0x86, 0x9b, 0x90, 0xa1, 0xaa, 0xb7, 0xbc, 0xd5, 0xde, 0xc3, 0xc8, 0xf9, 0xf2, 0xef, 0xe4, /* 6 */
    0x3d, 0x36, 0x2b, 0x20, 0x11, 0x1a, 0x07, 0x0c, 0x65, 0x6e, 0x73, 0x78, 0x49, 0x42, 0x5f, 0x54, /* 7 */
    0xf7, 0xfc, 0xe1, 0xea, 0xdb, 0xd0, 0xcd, 0xc6, 0xaf, 0xa4, 0xb9, 0xb2, 0x83, 0x88, 0x95, 0x9e, /* 8 */
    0x47, 0x4c, 0x51, 0x5a, 0x6b, 0x60, 0x7d, 0x76, 0x1f, 0x14, 0x09, 0x02, 0x33, 0x38, 0x25, 0x2e, /* 9 */
    0x8c, 0x87, 0x9a, 0x91, 0xa0, 0xab, 0xb6, 0xbd, 0xd4, 0xdf, 0xc2, 0xc9, 0xf8, 0xf3, 0xee, 0xe5, /* a */
    0x3c, 0x37, 0x2a, 0x21, 0x10, 0x1b, 0x06, 0x0d, 0x64, 0x6f, 0x72, 0x79, 0x48, 0x43, 0x5e, 0x55, /* b */
    0x01, 0x0a, 0x17, 0x1c, 0x2d, 0x26, 0x3b, 0x30, 0x59, 0x52, 0x4f, 0x44, 0x75, 0x7e, 0x63, 0x68, /* c */
    0xb1, 0xba, 0xa7, 0xac, 0x9d, 0x96, 0x8b, 0x80, 0xe9, 0xe2, 0xff, 0xf4, 0xc5, 0xce, 0xd3, 0xd8, /* d */
    0x7a, 0x71, 0x6c, 0x67, 0x56, 0x5d, 0x40, 0x4b, 0x22, 0x29, 0x34, 0x3f, 0x0e, 0x05, 0x18, 0x13, /* e */
    0xca, 0xc1, 0xdc, 0xd7, 0xe6, 0xed, 0xf0, 0xfb, 0x92, 0x99, 0x84, 0x8f, 0xbe, 0xb5, 0xa8, 0xa3, /* f */
};

/* ArrayIndex*{0d} */
static const UINT8 aes_mul_d[] = {
  /*  0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f    */
    0x00, 0x0d, 0x1a, 0x17, 0x34, 0x39, 0x2e, 0x23, 0x68, 0x65, 0x72, 0x7f, 0x5c, 0x51, 0x46, 0x4b, /* 0 */
    0xd0, 0xdd, 0xca, 0xc7, 0xe4, 0xe9, 0xfe, 0xf3, 0xb8, 0xb5, 0xa2, 0xaf, 0x8c, 0x81, 0x96, 0x9b, /* 1 */
    0xbb, 0xb6, 0xa1, 0xac, 0x8f, 0x82, 0x95, 0x98, 0xd3, 0xde, 0xc9, 0xc4, 0xe7, 0xea, 0xfd, 0xf0, /* 2 */
    0x6b, 0x66, 0x71, 0x7c, 0x5f, 0x52, 0x45, 0x48, 0x03, 0x0e, 0x19, 0x14, 0x37, 0x3a, 0x2d, 0x20, /* 3 */
    0x6d, 0x60, 0x77, 0x7a, 0x59, 0x54, 0x43, 0x4e, 0x05, 0x08, 0x1f, 0x12, 0x31, 0x3c, 0x2b, 0x26, /* 4 */
    0xbd, 0xb0, 0xa7, 0xaa, 0x89, 0x84, 0x93, 0x9e, 0xd5, 0xd8, 0xcf, 0xc2, 0xe1, 0xec, 0xfb, 0xf6, /* 5 */
    0xd6, 0xdb, 0xcc, 0xc1, 0xe2, 0xef, 0xf8, 0xf5, 0xbe, 0xb3, 0xa4, 0xa9, 0x8a, 0x87, 0x90, 0x9d, /* 6 */
    0x06, 0x0b, 0x1c, 0x11, 0x32, 0x3f, 0x28, 0x25, 0x6e, 0x63, 0x74, 0x79, 0x5a, 0x57, 0x40, 0x4d, /* 7 */
    0xda, 0xd7, 0xc0, 0xcd, 0xee, 0xe3, 0xf4, 0xf9, 0xb2, 0xbf, 0xa8, 0xa5, 0x86, 0x8b, 0x9c, 0x91, /* 8 */
    0x0a, 0x07, 0x10, 0x1d, 0x3e, 0x33, 0x24, 0x29, 0x62, 0x6f, 0x78, 0x75, 0x56, 0x5b, 0x4c, 0x41, /* 9 */
    0x61, 0x6c, 0x7b, 0x76, 0x55, 0x58, 0x4f, 0x42, 0x09, 0x04, 0x13, 0x1e, 0x3d, 0x30, 0x27, 0x2a, /* a */
    0xb1, 0xbc, 0xab, 0xa6, 0x85, 0x88, 0x9f, 0x92, 0xd9, 0xd4, 0xc3, 0xce, 0xed, 0xe0, 0xf7, 0xfa, /* b */
    0xb7, 0xba, 0xad, 0xa0, 0x83, 0x8e, 0x99, 0x94, 0xdf, 0xd2, 0xc5, 0xc8, 0xeb, 0xe6, 0xf1, 0xfc, /* c */
    0x67, 0x6a, 0x7d, 0x70, 0x53, 0x5e, 0x49, 0x44, 0x0f, 0x02, 0x15, 0x18, 0x3b, 0x36, 0x21, 0x2c, /* d */
    0x0c, 0x01, 0x16, 0x1b, 0x38, 0x35, 0x22, 0x2f, 0x64, 0x69, 0x7e, 0x73, 0x50, 0x5d, 0x4a, 0x47, /* e */
    0xdc, 0xd1, 0xc6, 0xcb, 0xe8, 0xe5, 0xf2, 0xff, 0xb4, 0xb9, 0xae, 0xa3, 0x80, 0x8d, 0x9a, 0x97, /* f */
};

/* ArrayIndex*{0e} */
static const UINT8 aes_mul_e[] = {
  /*  0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f    */
    0x00, 0x0e, 0x1c, 0x12, 0x38, 0x36, 0x24, 0x2a, 0x70, 0x7e, 0x6c, 0x62, 0x48, 0x46, 0x54, 0x5a, /* 0 */
    0xe0, 0xee, 0xfc, 0xf2, 0xd8, 0xd6, 0xc4, 0xca, 0x90, 0x9e, 0x8c, 0x82, 0xa8, 0xa6, 0xb4, 0xba, /* 1 */
    0xdb, 0xd5, 0xc7, 0xc9, 0xe3, 0xed, 0xff, 0xf1, 0xab, 0xa5, 0xb7, 0xb9, 0x93, 0x9d, 0x8f, 0x81, /* 2 */
    0x3b, 0x35, 0x27, 0x29, 0x03, 0x0d, 0x1f, 0x11, 0x4b, 0x45, 0x57, 0x59, 0x73, 0x7d, 0x6f, 0x61, /* 3 */
    0xad, 0xa3, 0xb1, 0xbf, 0x95, 0x9b, 0x89, 0x87, 0xdd, 0xd3, 0xc1, 0xcf, 0xe5, 0xeb, 0xf9, 0xf7, /* 4 */
    0x4d, 0x43, 0x51, 0x5f, 0x75, 0x7b, 0x69, 0x67, 0x3d, 0x33, 0x21, 0x2f, 0x05, 0x0b, 0x19, 0x17, /* 5 */
    0x76, 0x78, 0x6a, 0x64, 0x4e, 0x40, 0x52, 0x5c, 0x06, 0x08, 0x1a, 0x14, 0x3e, 0x30, 0x22, 0x2c, /* 6 */
    0x96, 0x98, 0x8a, 0x84, 0xae, 0xa0, 0xb2, 0xbc, 0xe6, 0xe8, 0xfa, 0xf4, 0xde, 0xd0, 0xc2, 0xcc, /* 7 */
    0x41, 0x4f, 0x5d, 0x53, 0x79, 0x77, 0x65, 0x6b, 0x31, 0x3f, 0x2d, 0x23, 0x09, 0x07, 0x15, 0x1b, /* 8 */
    0xa1, 0xaf, 0xbd, 0xb3, 0x99, 0x97, 0x85, 0x8b, 0xd1, 0xdf, 0xcd, 0xc3, 0xe9, 0xe7, 0xf5, 0xfb, /* 9 */
    0x9a, 0x94, 0x86, 0x88, 0xa2, 0xac, 0xbe, 0xb0, 0xea, 0xe4, 0xf6, 0xf8, 0xd2, 0xdc, 0xce, 0xc0, /* a */
    0x7a, 0x74, 0x66, 0x68, 0x42, 0x4c, 0x5e, 0x50, 0x0a, 0x04, 0x16, 0x18, 0x32, 0x3c, 0x2e, 0x20, /* b */
    0xec, 0xe2, 0xf0, 0xfe, 0xd4, 0xda, 0xc8, 0xc6, 0x9c, 0x92, 0x80, 0x8e, 0xa4, 0xaa, 0xb8, 0xb6, /* c */
    0x0c, 0x02, 0x10, 0x1e, 0x34, 0x3a, 0x28, 0x26, 0x7c, 0x72, 0x60, 0x6e, 0x44, 0x4a, 0x58, 0x56, /* d */
    0x37, 0x39, 0x2b, 0x25, 0x0f, 0x01, 0x13, 0x1d, 0x47, 0x49, 0x5b, 0x55, 0x7f, 0x71, 0x63, 0x6d, /* e */
    0xd7, 0xd9, 0xcb, 0xc5, 0xef, 0xe1, 0xf3, 0xfd, 0xa7, 0xa9, 0xbb, 0xb5, 0x9f, 0x91, 0x83, 0x8d, /* f */
};

/* For AES_CMAC */
#define AES_MAC_LENGTH 16 /* 128-bit string */
static UINT8 Const_Zero[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static UINT8 Const_Rb[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87};
   
/*
========================================================================
Routine Description:
    AES key expansion (key schedule)

Arguments:
    Key              Cipher key, it may be 16, 24, or 32 bytes (128, 192, or 256 bits)
    KeyLength        The length of cipher key in bytes
    paes_ctx         Pointer to AES_CTX_STRUC

Return Value:
    paes_ctx         Retrun the KeyWordExpansion of AES_CTX_STRUC

Note:
    Pseudo code for key expansion
    ------------------------------------------
       Nk = (key length/4);
       
       while (i < Nk)
           KeyWordExpansion[i] = word(key[4*i], key[4*i + 1], key[4*i + 2], key[4*i + 3]);
           i++;
       end while

       while (i < ((key length/4 + 6 + 1)*4) )
           temp = KeyWordExpansion[i - 1];
           if (i % Nk ==0)
               temp = SubWord(RotWord(temp)) ^ Rcon[i/Nk];
           else if ((Nk > 6) && (i % 4 == 4))
               temp = SubWord(temp);
           end if

           KeyWordExpansion[i] = KeyWordExpansion[i - Nk]^ temp;
           i++;
       end while
========================================================================
*/
VOID RT_AES_KeyExpansion (
    IN UINT8 Key[],
    IN UINT KeyLength,
    INOUT AES_CTX_STRUC *paes_ctx)
{
    UINT KeyIndex = 0;
    UINT NumberOfWordOfKey, NumberOfWordOfKeyExpansion;
    UINT8  TempWord[AES_KEY_ROWS], Temp;
    UINT32 Temprcon;

    NumberOfWordOfKey = KeyLength >> 2;
    while (KeyIndex < NumberOfWordOfKey)
    {
        paes_ctx->KeyWordExpansion[0][KeyIndex] = Key[4*KeyIndex];
        paes_ctx->KeyWordExpansion[1][KeyIndex] = Key[4*KeyIndex + 1];
        paes_ctx->KeyWordExpansion[2][KeyIndex] = Key[4*KeyIndex + 2];
        paes_ctx->KeyWordExpansion[3][KeyIndex] = Key[4*KeyIndex + 3];
        KeyIndex++;
    } /* End of while */

    NumberOfWordOfKeyExpansion = ((UINT) AES_KEY_ROWS) * ((KeyLength >> 2) + 6 + 1);    
    while (KeyIndex < NumberOfWordOfKeyExpansion)
    {
        TempWord[0] = paes_ctx->KeyWordExpansion[0][KeyIndex - 1];
        TempWord[1] = paes_ctx->KeyWordExpansion[1][KeyIndex - 1];
        TempWord[2] = paes_ctx->KeyWordExpansion[2][KeyIndex - 1];
        TempWord[3] = paes_ctx->KeyWordExpansion[3][KeyIndex - 1];
        if ((KeyIndex % NumberOfWordOfKey) == 0) {
            Temprcon = aes_rcon[KeyIndex/NumberOfWordOfKey];
            Temp = aes_sbox_enc[TempWord[1]]^((Temprcon >> 24) & 0xff);
            TempWord[1] = aes_sbox_enc[TempWord[2]]^((Temprcon >> 16) & 0xff);
            TempWord[2] = aes_sbox_enc[TempWord[3]]^((Temprcon >>  8) & 0xff);
            TempWord[3] = aes_sbox_enc[TempWord[0]]^((Temprcon      ) & 0xff);
            TempWord[0] = Temp;
        } else if ((NumberOfWordOfKey > 6) && ((KeyIndex % NumberOfWordOfKey) == 4)) {
            Temp = aes_sbox_enc[TempWord[0]];
            TempWord[1] = aes_sbox_enc[TempWord[1]];
            TempWord[2] = aes_sbox_enc[TempWord[2]];
            TempWord[3] = aes_sbox_enc[TempWord[3]];
            TempWord[0] = Temp;
        }
        paes_ctx->KeyWordExpansion[0][KeyIndex] = paes_ctx->KeyWordExpansion[0][KeyIndex - NumberOfWordOfKey]^TempWord[0];
        paes_ctx->KeyWordExpansion[1][KeyIndex] = paes_ctx->KeyWordExpansion[1][KeyIndex - NumberOfWordOfKey]^TempWord[1];
        paes_ctx->KeyWordExpansion[2][KeyIndex] = paes_ctx->KeyWordExpansion[2][KeyIndex - NumberOfWordOfKey]^TempWord[2];
        paes_ctx->KeyWordExpansion[3][KeyIndex] = paes_ctx->KeyWordExpansion[3][KeyIndex - NumberOfWordOfKey]^TempWord[3];
        KeyIndex++;
    } /* End of while */
} /* End of RT_AES_KeyExpansion */


/*
========================================================================
Routine Description:
    AES encryption

Arguments:
    PlainBlock       The block of plain text, 16 bytes(128 bits) each block
    PlainBlockSize   The length of block of plain text in bytes
    Key              Cipher key, it may be 16, 24, or 32 bytes (128, 192, or 256 bits)
    KeyLength        The length of cipher key in bytes
    CipherBlockSize  The length of allocated cipher block in bytes

Return Value:
    CipherBlock      Return cipher text
    CipherBlockSize  Return the length of real used cipher block in bytes

Note:
    Reference to FIPS-PUB 197
    1. Check if block size is 16 bytes(128 bits) and if key length is 16, 24, or 32 bytes(128, 192, or 256 bits)
    2. Transfer the plain block to state block 
    3. Main encryption rounds
    4. Transfer the state block to cipher block
    ------------------------------------------
       NumberOfRound = (key length / 4) + 6;
       state block = plain block;
       
       AddRoundKey(state block, key);
       for round = 1 to NumberOfRound
           SubBytes(state block)
           ShiftRows(state block)
           MixColumns(state block)
           AddRoundKey(state block, key);
       end for

       SubBytes(state block)
       ShiftRows(state block)
       AddRoundKey(state block, key);

       cipher block = state block;
========================================================================
*/
VOID RT_AES_Encrypt (
    IN UINT8 PlainBlock[],
    IN UINT PlainBlockSize,
    IN UINT8 Key[],
    IN UINT KeyLength,
    OUT UINT8 CipherBlock[],
    INOUT UINT *CipherBlockSize)
{
/*    AES_CTX_STRUC aes_ctx;
*/
	AES_CTX_STRUC *paes_ctx = NULL;
    UINT RowIndex, ColumnIndex;
    UINT RoundIndex, NumberOfRound = 0;
    UINT8 Temp, Row0, Row1, Row2, Row3;

    /*   
     * 1. Check if block size is 16 bytes(128 bits) and if key length is 16, 24, or 32 bytes(128, 192, or 256 bits) 
     */
    if (PlainBlockSize != AES_BLOCK_SIZES) {
    	DBGPRINT(RT_DEBUG_ERROR, ("RT_AES_Encrypt: plain block size is %d bytes, it must be %d bytes(128 bits).\n", 
            PlainBlockSize, AES_BLOCK_SIZES));
        return;
    } /* End of if */
    if ((KeyLength != AES_KEY128_LENGTH) && (KeyLength != AES_KEY192_LENGTH) && (KeyLength != AES_KEY256_LENGTH)) {
    	DBGPRINT(RT_DEBUG_ERROR, ("RT_AES_Encrypt: key length is %d bytes, it must be %d, %d, or %d bytes(128, 192, or 256 bits).\n", 
            KeyLength, AES_KEY128_LENGTH, AES_KEY192_LENGTH, AES_KEY256_LENGTH));
        return;
    } /* End of if */
    if (*CipherBlockSize < AES_BLOCK_SIZES) {
    	DBGPRINT(RT_DEBUG_ERROR, ("RT_AES_Encrypt: cipher block size is %d bytes, it must be %d bytes(128 bits).\n", 
            *CipherBlockSize, AES_BLOCK_SIZES));
        return;
    } /* End of if */

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&paes_ctx, sizeof(AES_CTX_STRUC));
	if (paes_ctx == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return;
	}

    /* 
     * 2. Transfer the plain block to state block 
     */
    for (RowIndex = 0; RowIndex < AES_STATE_ROWS;RowIndex++)
        for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
            paes_ctx->State[RowIndex][ColumnIndex] = PlainBlock[RowIndex + 4*ColumnIndex];

    /* 
     *  3. Main encryption rounds
     */
    RT_AES_KeyExpansion(Key, KeyLength, paes_ctx);
    NumberOfRound = (KeyLength >> 2) + 6;

    /* AES_AddRoundKey */
    RoundIndex = 0;
    for (RowIndex = 0; RowIndex < AES_STATE_ROWS;RowIndex++)
        for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
            paes_ctx->State[RowIndex][ColumnIndex] ^= paes_ctx->KeyWordExpansion[RowIndex][(RoundIndex*((UINT) AES_STATE_COLUMNS)) + ColumnIndex];

    for (RoundIndex = 1; RoundIndex < NumberOfRound;RoundIndex++)
    {
        /* AES_SubBytes */
        for (RowIndex = 0; RowIndex < AES_STATE_ROWS;RowIndex++)
            for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
                paes_ctx->State[RowIndex][ColumnIndex] = aes_sbox_enc[paes_ctx->State[RowIndex][ColumnIndex]];

        /* AES_ShiftRows */
        Temp = paes_ctx->State[1][0];
        paes_ctx->State[1][0] = paes_ctx->State[1][1];
        paes_ctx->State[1][1] = paes_ctx->State[1][2];
        paes_ctx->State[1][2] = paes_ctx->State[1][3];
        paes_ctx->State[1][3] = Temp;
        Temp = paes_ctx->State[2][0];
        paes_ctx->State[2][0] = paes_ctx->State[2][2];
        paes_ctx->State[2][2] = Temp;
        Temp = paes_ctx->State[2][1];
        paes_ctx->State[2][1] = paes_ctx->State[2][3];
        paes_ctx->State[2][3] = Temp;
        Temp = paes_ctx->State[3][3];
        paes_ctx->State[3][3] = paes_ctx->State[3][2];
        paes_ctx->State[3][2] = paes_ctx->State[3][1];
        paes_ctx->State[3][1] = paes_ctx->State[3][0];
        paes_ctx->State[3][0] = Temp;

        /* AES_MixColumns */
        for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
        {
            Row0 = paes_ctx->State[0][ColumnIndex];
            Row1 = paes_ctx->State[1][ColumnIndex];
            Row2 = paes_ctx->State[2][ColumnIndex];
            Row3 = paes_ctx->State[3][ColumnIndex];
            paes_ctx->State[0][ColumnIndex] = aes_mul_2[Row0]^aes_mul_3[Row1]^Row2^Row3;
            paes_ctx->State[1][ColumnIndex] = Row0^aes_mul_2[Row1]^aes_mul_3[Row2]^Row3;
            paes_ctx->State[2][ColumnIndex] = Row0^Row1^aes_mul_2[Row2]^aes_mul_3[Row3];
            paes_ctx->State[3][ColumnIndex] = aes_mul_3[Row0]^Row1^Row2^aes_mul_2[Row3];
        }

        /* AES_AddRoundKey */
        for (RowIndex = 0; RowIndex < AES_STATE_ROWS;RowIndex++)
            for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
                paes_ctx->State[RowIndex][ColumnIndex] ^= paes_ctx->KeyWordExpansion[RowIndex][(RoundIndex*((UINT) AES_STATE_COLUMNS)) + ColumnIndex];
    } /* End of for */

    /* AES_SubBytes */
    for (RowIndex = 0; RowIndex < AES_STATE_ROWS;RowIndex++)
        for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
            paes_ctx->State[RowIndex][ColumnIndex] = aes_sbox_enc[paes_ctx->State[RowIndex][ColumnIndex]];
    /* AES_ShiftRows */
    Temp = paes_ctx->State[1][0];
    paes_ctx->State[1][0] = paes_ctx->State[1][1];
    paes_ctx->State[1][1] = paes_ctx->State[1][2];
    paes_ctx->State[1][2] = paes_ctx->State[1][3];
    paes_ctx->State[1][3] = Temp;
    Temp = paes_ctx->State[2][0];
    paes_ctx->State[2][0] = paes_ctx->State[2][2];
    paes_ctx->State[2][2] = Temp;
    Temp = paes_ctx->State[2][1];
    paes_ctx->State[2][1] = paes_ctx->State[2][3];
    paes_ctx->State[2][3] = Temp;
    Temp = paes_ctx->State[3][3];
    paes_ctx->State[3][3] = paes_ctx->State[3][2];
    paes_ctx->State[3][2] = paes_ctx->State[3][1];
    paes_ctx->State[3][1] = paes_ctx->State[3][0];
    paes_ctx->State[3][0] = Temp;
    /* AES_AddRoundKey */
    for (RowIndex = 0; RowIndex < AES_STATE_ROWS;RowIndex++)
        for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
            paes_ctx->State[RowIndex][ColumnIndex] ^= paes_ctx->KeyWordExpansion[RowIndex][(RoundIndex*((UINT) AES_STATE_COLUMNS)) + ColumnIndex];

    /* 
     * 4. Transfer the state block to cipher block 
     */
    for (RowIndex = 0; RowIndex < AES_STATE_ROWS;RowIndex++)
        for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
            CipherBlock[RowIndex + 4*ColumnIndex] = paes_ctx->State[RowIndex][ColumnIndex];

    *CipherBlockSize = ((UINT) AES_STATE_ROWS)*((UINT) AES_STATE_COLUMNS);

	if (paes_ctx != NULL)
		os_free_mem(NULL, paes_ctx);
} /* End of RT_AES_Encrypt */


/*
========================================================================
Routine Description:
    AES decryption

Arguments:
    CipherBlock      The block of cipher text, 16 bytes(128 bits) each block
    CipherBlockSize  The length of block of cipher text in bytes
    Key              Cipher key, it may be 16, 24, or 32 bytes (128, 192, or 256 bits)
    KeyLength        The length of cipher key in bytes
    PlainBlockSize   The length of allocated plain block in bytes

Return Value:
    PlainBlock       Return plain text
    PlainBlockSize  Return the length of real used plain block in bytes

Note:
    Reference to FIPS-PUB 197
    1. Check if block size is 16 bytes(128 bits) and if key length is 16, 24, or 32 bytes(128, 192, or 256 bits)
    2. Transfer the cipher block to state block 
    3. Main decryption rounds
    4. Transfer the state block to plain block
    ------------------------------------------
       NumberOfRound = (key length / 4) + 6;
       state block = cipher block;
       
       AddRoundKey(state block, key);
       for round = NumberOfRound to 1
           InvSubBytes(state block)
           InvShiftRows(state block)
           InvMixColumns(state block)
           AddRoundKey(state block, key);
       end for

       InvSubBytes(state block)
       InvShiftRows(state block)
       AddRoundKey(state block, key);

       plain block = state block;
========================================================================
*/
VOID RT_AES_Decrypt (
    IN UINT8 CipherBlock[],
    IN UINT CipherBlockSize,
    IN UINT8 Key[],
    IN UINT KeyLength,
    OUT UINT8 PlainBlock[],
    INOUT UINT *PlainBlockSize)
{
/*    AES_CTX_STRUC aes_ctx;
*/
	AES_CTX_STRUC *paes_ctx = NULL;
    UINT RowIndex, ColumnIndex;
    UINT RoundIndex, NumberOfRound = 0;
    UINT8 Temp, Row0, Row1, Row2, Row3;

    /*   
     * 1. Check if block size is 16 bytes(128 bits) and if key length is 16, 24, or 32 bytes(128, 192, or 256 bits) 
     */
    if (*PlainBlockSize < AES_BLOCK_SIZES) {
    	DBGPRINT(RT_DEBUG_ERROR, ("RT_AES_Decrypt: plain block size is %d bytes, it must be %d bytes(128 bits).\n", 
            *PlainBlockSize, AES_BLOCK_SIZES));
        return;
    } /* End of if */
    if ((KeyLength != AES_KEY128_LENGTH) && (KeyLength != AES_KEY192_LENGTH) && (KeyLength != AES_KEY256_LENGTH)) {
    	DBGPRINT(RT_DEBUG_ERROR, ("RT_AES_Decrypt: key length is %d bytes, it must be %d, %d, or %d bytes(128, 192, or 256 bits).\n", 
            KeyLength, AES_KEY128_LENGTH, AES_KEY192_LENGTH, AES_KEY256_LENGTH));
        return;
    } /* End of if */
    if (CipherBlockSize != AES_BLOCK_SIZES) {
    	DBGPRINT(RT_DEBUG_ERROR, ("RT_AES_Decrypt: cipher block size is %d bytes, it must be %d bytes(128 bits).\n", 
            CipherBlockSize, AES_BLOCK_SIZES));
        return;
    } /* End of if */

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&paes_ctx, sizeof(AES_CTX_STRUC));
	if (paes_ctx == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return;
	}

    /* 
     * 2. Transfer the cipher block to state block 
     */
    for (RowIndex = 0; RowIndex < AES_STATE_ROWS;RowIndex++)
        for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
            paes_ctx->State[RowIndex][ColumnIndex] = CipherBlock[RowIndex + 4*ColumnIndex];

    /* 
     *  3. Main decryption rounds
     */
    RT_AES_KeyExpansion(Key, KeyLength, paes_ctx);
    NumberOfRound = (KeyLength >> 2) + 6;

    /* AES_AddRoundKey */
    RoundIndex = NumberOfRound;
    for (RowIndex = 0; RowIndex < AES_STATE_ROWS;RowIndex++)
        for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
            paes_ctx->State[RowIndex][ColumnIndex] ^= paes_ctx->KeyWordExpansion[RowIndex][(RoundIndex*((UINT) AES_STATE_COLUMNS)) + ColumnIndex];

    for (RoundIndex = (NumberOfRound - 1); RoundIndex > 0 ;RoundIndex--)
    {
        /* AES_InvShiftRows */
        Temp = paes_ctx->State[1][3];
        paes_ctx->State[1][3] = paes_ctx->State[1][2];
        paes_ctx->State[1][2] = paes_ctx->State[1][1];
        paes_ctx->State[1][1] = paes_ctx->State[1][0];
        paes_ctx->State[1][0] = Temp;
        Temp = paes_ctx->State[2][0];
        paes_ctx->State[2][0] = paes_ctx->State[2][2];
        paes_ctx->State[2][2] = Temp;
        Temp = paes_ctx->State[2][1];
        paes_ctx->State[2][1] = paes_ctx->State[2][3];
        paes_ctx->State[2][3] = Temp;
        Temp = paes_ctx->State[3][0];
        paes_ctx->State[3][0] = paes_ctx->State[3][1];
        paes_ctx->State[3][1] = paes_ctx->State[3][2];
        paes_ctx->State[3][2] = paes_ctx->State[3][3];
        paes_ctx->State[3][3] = Temp;

        /* AES_InvSubBytes */
        for (RowIndex = 0; RowIndex < AES_STATE_ROWS;RowIndex++)
            for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
                paes_ctx->State[RowIndex][ColumnIndex] = aes_sbox_dec[paes_ctx->State[RowIndex][ColumnIndex]];

        /* AES_AddRoundKey */
        for (RowIndex = 0; RowIndex < AES_STATE_ROWS;RowIndex++)
            for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
                paes_ctx->State[RowIndex][ColumnIndex] ^= paes_ctx->KeyWordExpansion[RowIndex][(RoundIndex*((UINT) AES_STATE_COLUMNS)) + ColumnIndex];

        /* AES_InvMixColumns */
        for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
        {
            Row0 = paes_ctx->State[0][ColumnIndex];
            Row1 = paes_ctx->State[1][ColumnIndex];
            Row2 = paes_ctx->State[2][ColumnIndex];
            Row3 = paes_ctx->State[3][ColumnIndex];
            paes_ctx->State[0][ColumnIndex] = aes_mul_e[Row0]^aes_mul_b[Row1]^aes_mul_d[Row2]^aes_mul_9[Row3];
            paes_ctx->State[1][ColumnIndex] = aes_mul_9[Row0]^aes_mul_e[Row1]^aes_mul_b[Row2]^aes_mul_d[Row3];
            paes_ctx->State[2][ColumnIndex] = aes_mul_d[Row0]^aes_mul_9[Row1]^aes_mul_e[Row2]^aes_mul_b[Row3];
            paes_ctx->State[3][ColumnIndex] = aes_mul_b[Row0]^aes_mul_d[Row1]^aes_mul_9[Row2]^aes_mul_e[Row3];
        }
    } /* End of for */

    /* AES_InvShiftRows */
    Temp = paes_ctx->State[1][3];
    paes_ctx->State[1][3] = paes_ctx->State[1][2];
    paes_ctx->State[1][2] = paes_ctx->State[1][1];
    paes_ctx->State[1][1] = paes_ctx->State[1][0];
    paes_ctx->State[1][0] = Temp;
    Temp = paes_ctx->State[2][0];
    paes_ctx->State[2][0] = paes_ctx->State[2][2];
    paes_ctx->State[2][2] = Temp;
    Temp = paes_ctx->State[2][1];
    paes_ctx->State[2][1] = paes_ctx->State[2][3];
    paes_ctx->State[2][3] = Temp;
    Temp = paes_ctx->State[3][0];
    paes_ctx->State[3][0] = paes_ctx->State[3][1];
    paes_ctx->State[3][1] = paes_ctx->State[3][2];
    paes_ctx->State[3][2] = paes_ctx->State[3][3];
    paes_ctx->State[3][3] = Temp;
    /* AES_InvSubBytes */
    for (RowIndex = 0; RowIndex < AES_STATE_ROWS;RowIndex++)
        for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
            paes_ctx->State[RowIndex][ColumnIndex] = aes_sbox_dec[paes_ctx->State[RowIndex][ColumnIndex]];
    /* AES_AddRoundKey */
    for (RowIndex = 0; RowIndex < AES_STATE_ROWS;RowIndex++)
        for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
            paes_ctx->State[RowIndex][ColumnIndex] ^= paes_ctx->KeyWordExpansion[RowIndex][(RoundIndex*((UINT) AES_STATE_COLUMNS)) + ColumnIndex];

    /* 
     * 4. Transfer the state block to plain block 
     */
    for (RowIndex = 0; RowIndex < AES_STATE_ROWS;RowIndex++)
        for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
            PlainBlock[RowIndex + 4*ColumnIndex] = paes_ctx->State[RowIndex][ColumnIndex];

    *PlainBlockSize = ((UINT) AES_STATE_ROWS)*((UINT) AES_STATE_COLUMNS);

	if (paes_ctx != NULL)
		os_free_mem(NULL, paes_ctx);
} /* End of RT_AES_Decrypt */



/*
========================================================================
Routine Description:
    AES-CBCMAC 

Arguments:
    Payload        Data
    PayloadLength  The length of data in bytes
    Key              Cipher key
    KeyLength        The length of cipher key in bytes depend on block cipher (16, 24, or 32 bytes)
    Nonce            Nonce
    NonceLength      The length of nonce in bytes
    AAD              Additional authenticated data
    AADLength        The length of AAD in bytes
    MACLength        The length of MAC in bytes

Return Value:
    MACText       The mac

Note:
    Reference to RFC 3601, and NIST 800-38C.
========================================================================
*/
VOID AES_CCM_MAC (
    IN UINT8 Payload[],
    IN UINT  PayloadLength,
    IN UINT8 Key[],
    IN UINT  KeyLength,
    IN UINT8 Nonce[],
    IN UINT  NonceLength,
    IN UINT8 AAD[],
    IN UINT  AADLength,
    IN UINT  MACLength,
    OUT UINT8 MACText[])
{
    UINT8 Block[AES_BLOCK_SIZES], Block_MAC[AES_BLOCK_SIZES];
    UINT  Block_Index = 0, ADD_Index = 0, Payload_Index = 0;
    UINT  Temp_Value = 0, Temp_Index = 0, Temp_Length = 0, Copy_Length = 0;

    /*   
     * 1. Formatting of the Control Information and the Nonce
     */
    NdisZeroMemory(Block, AES_BLOCK_SIZES);
    if (AADLength > 0)
        Block[0] |= 0x40; /* Set bit 6 to 1 */
    Temp_Value = ((MACLength - 2) >> 1) << 3; /* Set bit 3-5 to (t-2)/2 */
    Block[0] |= Temp_Value;
    Temp_Value = (15 - NonceLength) - 1; /* Set bit 0-2 to (q-1), q = 15 - Nonce Length */
    Block[0] |= Temp_Value;
    for (Temp_Index = 0; Temp_Index < NonceLength; Temp_Index++)
        Block[Temp_Index + 1] = Nonce[Temp_Index];
    if (NonceLength < 12)
        Block[12] = (PayloadLength >> 24) & 0xff;
    if (NonceLength < 13)
        Block[13] = (PayloadLength >> 16) & 0xff;
    Block[14] = (PayloadLength >> 8) & 0xff;
    Block[15] = PayloadLength & 0xff;

    NdisZeroMemory(Block_MAC, AES_BLOCK_SIZES);
    Temp_Length = sizeof(Block_MAC);
    RT_AES_Encrypt(Block, AES_BLOCK_SIZES , Key, KeyLength, Block_MAC, &Temp_Length);

    /*
     * 2. Formatting of the Associated Data
     *      If 0 < AADLength < (2^16 - 2^8), AData_Length = 2
     *      If (2^16 - 2^8) < AADLength < 2^32, AData_Length = 6
     *      If 2^32 < AADLength < 2^64, AData_Length = 10 (not implement)
     */    
    NdisZeroMemory(Block, AES_BLOCK_SIZES);
    if ((AADLength > 0) && (AADLength < 0xFF00)) {
        Block_Index = 2;
        Block[0] = (AADLength >> 8) & 0xff;
        Block[1] = AADLength & 0xff;
    } else {
        Block_Index = 6;
        Block[2] = (AADLength >> 24) & 0xff;
        Block[3] = (AADLength >> 16) & 0xff;
        Block[4] = (AADLength >> 8) & 0xff;
        Block[5] = AADLength & 0xff;
    } /* End of if */

    while (ADD_Index < AADLength) 
    {
        Copy_Length = AADLength - ADD_Index;
        if (Copy_Length > AES_BLOCK_SIZES)
            Copy_Length = AES_BLOCK_SIZES;
        if ((Copy_Length + Block_Index) > AES_BLOCK_SIZES) {
            Copy_Length = AES_BLOCK_SIZES - Block_Index;
        } /* End of if */                    
        for (Temp_Index = 0; Temp_Index < Copy_Length; Temp_Index++)
            Block[Temp_Index + Block_Index] = AAD[ADD_Index + Temp_Index];        
        for (Temp_Index = 0; Temp_Index < AES_BLOCK_SIZES; Temp_Index++)
            Block[Temp_Index] ^= Block_MAC[Temp_Index];
        NdisZeroMemory(Block_MAC, AES_BLOCK_SIZES);
        Temp_Length = sizeof(Block_MAC);        
        RT_AES_Encrypt(Block, AES_BLOCK_SIZES , Key, KeyLength, Block_MAC, &Temp_Length);
        ADD_Index += Copy_Length;
        Block_Index = 0;        
        NdisZeroMemory(Block, AES_BLOCK_SIZES);
    } /* End of while */

    /*
     * 3. Calculate the MAC (MIC)
     */
    while (Payload_Index < PayloadLength) 
    {
        NdisZeroMemory(Block, AES_BLOCK_SIZES);
        Copy_Length = PayloadLength - Payload_Index;
        if (Copy_Length > AES_BLOCK_SIZES)
            Copy_Length = AES_BLOCK_SIZES;
        for (Temp_Index = 0; Temp_Index < Copy_Length; Temp_Index++)
            Block[Temp_Index] = Payload[Payload_Index + Temp_Index];        
        for (Temp_Index = 0; Temp_Index < AES_BLOCK_SIZES; Temp_Index++)
            Block[Temp_Index] ^= Block_MAC[Temp_Index];
        NdisZeroMemory(Block_MAC, AES_BLOCK_SIZES);
        Temp_Length = sizeof(Block_MAC);
        RT_AES_Encrypt(Block, AES_BLOCK_SIZES , Key, KeyLength, Block_MAC, &Temp_Length);
        Payload_Index += Copy_Length;
    } /* End of while */
    for (Temp_Index = 0; Temp_Index < MACLength; Temp_Index++)
        MACText[Temp_Index] = Block_MAC[Temp_Index];
} /* End of AES_CCM_MAC */


/*
========================================================================
Routine Description:
    AES-CBCMAC Encryption

Arguments:
    PlainText        Plain text
    PlainTextLength  The length of plain text in bytes
    Key              Cipher key
    KeyLength        The length of cipher key in bytes depend on block cipher (16, 24, or 32 bytes)
    Nonce            Nonce
    NonceLength      The length of nonce in bytes
    AAD              Additional authenticated data
    AADLength        The length of AAD in bytes
    MACLength        The length of MAC in bytes
    CipherTextLength    The length of allocated memory spaces in bytes

Return Value:
    CipherText       The ciphertext
    CipherTextLength Return the length of the ciphertext in bytes

Function Value:
     0: Success
    -1: The key length must be 16 bytes.
    -2: A valid nonce length is 7-13 bytes.
    -3: The MAC length  must be 4, 6, 8, 10, 12, 14, or 16 bytes.
    -4: The CipherTextLength is not enough.

Note:
    Reference to RFC 3601, and NIST 800-38C.
    Here, the implement of AES_CCM is suitable for WI_FI.
========================================================================
*/
INT AES_CCM_Encrypt (
    IN UINT8 PlainText[],
    IN UINT PlainTextLength,
    IN UINT8 Key[],
    IN UINT KeyLength,
    IN UINT8 Nonce[],
    IN UINT NonceLength,
    IN UINT8 AAD[],
    IN UINT AADLength,
    IN UINT MACLength,
    OUT UINT8 CipherText[],
    INOUT UINT *CipherTextLength)
{
    UINT8 Block_MAC[AES_BLOCK_SIZES];
    UINT8 Block_CTR[AES_BLOCK_SIZES], Block_CTR_Cipher[AES_BLOCK_SIZES];
    UINT  Cipher_Index = 0;
    UINT Temp_Value = 0, Temp_Index = 0, Temp_Length = 0, Copy_Length = 0;

    /*   
     * 1. Check Input Values
     *    - Key length must be 16 bytes
     *    - Nonce length range is form 7 to 13 bytes
     *    - MAC length must be 4, 6, 8, 10, 12, 14, or 16 bytes
     *    - CipherTextLength > PlainTextLength + MACLength
     */
    if (KeyLength != AES_KEY128_LENGTH) {
    	DBGPRINT(RT_DEBUG_ERROR, ("AES_CCM_Encrypt: The key length must be %d bytes\n", AES_KEY128_LENGTH));
        return -1;
    } /* End of if */

    if ((NonceLength < 7) || (NonceLength > 13)) {
    	DBGPRINT(RT_DEBUG_ERROR, ("AES_CCM_Encrypt: A valid nonce length is 7-13 bytes\n"));
        return -2;
    } /* End of if */

    if ((MACLength != 4) && (MACLength != 6) && (MACLength != 8) && (MACLength != 10)
        && (MACLength != 12) && (MACLength != 14) && (MACLength != 16)) {
    	DBGPRINT(RT_DEBUG_ERROR, ("AES_CCM_Encrypt: The MAC length  must be 4, 6, 8, 10, 12, 14, or 16 bytes\n"));
        return -3;
    } /* End of if */

    if (*CipherTextLength < (PlainTextLength + MACLength)) {
    	DBGPRINT(RT_DEBUG_ERROR, ("AES_CCM_Encrypt: The CipherTextLength is not enough.\n"));
        return -4;
    } /* End of if */


    /*   
     * 1. Formatting of the Counter Block
     */
    NdisZeroMemory(Block_CTR, AES_BLOCK_SIZES);
    Temp_Value = (15 - NonceLength) - 1; /* Set bit 0-2 to (q-1), q = 15 - Nonce Length */
    Block_CTR[0] |= Temp_Value;
    for (Temp_Index = 0; Temp_Index < NonceLength; Temp_Index++)
        Block_CTR[Temp_Index + 1] = Nonce[Temp_Index];

    /*
     * 2. Calculate the MAC (MIC)
     */
    AES_CCM_MAC(PlainText, PlainTextLength, Key, KeyLength, Nonce, NonceLength, AAD, AADLength, MACLength, Block_MAC);
    Temp_Length = sizeof(Block_CTR_Cipher);
    RT_AES_Encrypt(Block_CTR, AES_BLOCK_SIZES , Key, KeyLength, Block_CTR_Cipher, &Temp_Length);
    for (Temp_Index = 0; Temp_Index < MACLength; Temp_Index++)
        Block_MAC[Temp_Index] ^= Block_CTR_Cipher[Temp_Index];

    /*   
     * 3. Cipher Payload
     */
    while (Cipher_Index < PlainTextLength) 
    {
        Block_CTR[15] += 1;
        Temp_Length = sizeof(Block_CTR_Cipher);
        RT_AES_Encrypt(Block_CTR, AES_BLOCK_SIZES , Key, KeyLength, Block_CTR_Cipher, &Temp_Length);

        Copy_Length = PlainTextLength - Cipher_Index;
        if (Copy_Length > AES_BLOCK_SIZES)
            Copy_Length = AES_BLOCK_SIZES;                
        for (Temp_Index = 0; Temp_Index < Copy_Length; Temp_Index++)
            CipherText[Cipher_Index + Temp_Index] = PlainText[Cipher_Index + Temp_Index]^Block_CTR_Cipher[Temp_Index];

        Cipher_Index += Copy_Length;
    } /* End of while */
    for (Temp_Index = 0; Temp_Index < MACLength; Temp_Index++)
            CipherText[PlainTextLength + Temp_Index] = Block_MAC[Temp_Index];    
    *CipherTextLength = PlainTextLength + MACLength;

    return 0;
} /* End of AES_CCM_Encrypt */


/*
========================================================================
Routine Description:
    AES-CBCMAC Decryption

Arguments:
    CipherText       The ciphertext
    CipherTextLength The length of cipher text in bytes
    Key              Cipher key
    KeyLength        The length of cipher key in bytes depend on block cipher (16, 24, or 32 bytes)
    Nonce            Nonce
    NonceLength      The length of nonce in bytes
    AAD              Additional authenticated data
    AADLength        The length of AAD in bytes
    CipherTextLength    The length of allocated memory spaces in bytes

Return Value:
    PlainText        Plain text
    PlainTextLength  Return the length of the plain text in bytes

Function Value:
     0: Success
    -1: The key length must be 16 bytes.
    -2: A valid nonce length is 7-13 bytes.
    -3: The MAC length  must be 4, 6, 8, 10, 12, 14, or 16 bytes.
    -4: The PlainTextLength is not enough.
    -5: The MIC does not match.
    
Note:
    Reference to RFC 3601, and NIST 800-38C.
    Here, the implement of AES_CCM is suitable for WI_FI.
========================================================================
*/
INT AES_CCM_Decrypt (
    IN UINT8 CipherText[],
    IN UINT  CipherTextLength,
    IN UINT8 Key[],
    IN UINT  KeyLength,
    IN UINT8 Nonce[],
    IN UINT  NonceLength,
    IN UINT8 AAD[],
    IN UINT  AADLength,
    IN UINT  MACLength,    
    OUT UINT8 PlainText[],
    INOUT UINT *PlainTextLength)
{
    UINT8 Block_MAC[AES_BLOCK_SIZES], Block_MAC_From_Cipher[AES_BLOCK_SIZES];
    UINT8 Block_CTR[AES_BLOCK_SIZES], Block_CTR_Cipher[AES_BLOCK_SIZES];
    UINT  Block_Index = 0, Cipher_Index = 0;
    UINT Temp_Value = 0, Temp_Index = 0, Temp_Length = 0, Copy_Length = 0;


    /*   
     * 1. Check Input Values
     *    - Key length must be 16 bytes
     *    - Nonce length range is form 7 to 13 bytes
     */
    if (KeyLength != AES_KEY128_LENGTH) {
    	DBGPRINT(RT_DEBUG_ERROR, ("AES_CCM_Decrypt: The key length must be %d bytes\n", AES_KEY128_LENGTH));
        return -1;
    } /* End of if */

    if ((NonceLength < 7) || (NonceLength > 13)) {
    	DBGPRINT(RT_DEBUG_ERROR, ("AES_CCM_Decrypt: A valid nonce length is 7-13 bytes\n"));
        return -2;
    } /* End of if */
    
    if ((MACLength != 4) && (MACLength != 6) && (MACLength != 8) && (MACLength != 10)
        && (MACLength != 12) && (MACLength != 14) && (MACLength != 16)) {
    	DBGPRINT(RT_DEBUG_ERROR, ("AES_CCM_Decrypt: The MAC length  must be 4, 6, 8, 10, 12, 14, or 16 bytes\n"));
        return -3;
    } /* End of if */
    
    if (*PlainTextLength < (CipherTextLength - MACLength)) {
    	DBGPRINT(RT_DEBUG_ERROR, ("AES_CCM_Decrypt: The PlainTextLength is not enough.\n"));
        return -4;
    } /* End of if */

    /*   
     * 2. Formatting of the Counter Block
     */
    NdisZeroMemory(Block_CTR, AES_BLOCK_SIZES);
    Temp_Value = (15 - NonceLength) - 1; /* Set bit 0-2 to (q-1), q = 15 - Nonce Length */
    Block_CTR[0] |= Temp_Value;
    for (Temp_Index = 0; Temp_Index < NonceLength; Temp_Index++)
        Block_CTR[Temp_Index + 1] = Nonce[Temp_Index];
    Temp_Length = sizeof(Block_CTR_Cipher);
    RT_AES_Encrypt(Block_CTR, AES_BLOCK_SIZES , Key, KeyLength, Block_CTR_Cipher, &Temp_Length);

    /*
     * 3. Catch the MAC (MIC) from CipherText
     */
    Block_Index = 0;
    for (Temp_Index = (CipherTextLength - MACLength); Temp_Index < CipherTextLength; Temp_Index++, Block_Index++)
        Block_MAC_From_Cipher[Block_Index] = CipherText[Temp_Index]^Block_CTR_Cipher[Block_Index];

    /*
     * 4. Decryption the Payload
     */     
    while (Cipher_Index < (CipherTextLength - MACLength)) 
    {
        Block_CTR[15] += 1;
        Temp_Length = sizeof(Block_CTR_Cipher);
        RT_AES_Encrypt(Block_CTR, AES_BLOCK_SIZES , Key, KeyLength, Block_CTR_Cipher, &Temp_Length);

        Copy_Length = (CipherTextLength - MACLength) - Cipher_Index;
        if (Copy_Length > AES_BLOCK_SIZES)
            Copy_Length = AES_BLOCK_SIZES;                
        for (Temp_Index = 0; Temp_Index < Copy_Length; Temp_Index++)
            PlainText[Cipher_Index + Temp_Index] = CipherText[Cipher_Index + Temp_Index]^Block_CTR_Cipher[Temp_Index];
        Cipher_Index += Copy_Length;
    } /* End of while */
    *PlainTextLength = CipherTextLength - MACLength;
    
    /*
     * 5. Calculate the MAC (MIC) from Payload
     */
    AES_CCM_MAC(PlainText, *PlainTextLength, Key, KeyLength, Nonce, NonceLength, AAD, AADLength, MACLength, Block_MAC);

    /*
     * 6. Check the MIC
     */
    if (NdisCmpMemory(Block_MAC_From_Cipher, Block_MAC, MACLength) != 0) {
    	DBGPRINT(RT_DEBUG_ERROR, ("AES_CCM_Decrypt: The MIC does not match.\n"));
        return -5;
    } /* End of if */

    return 0;
} /* End of AES_CCM_Decrypt */


/*
========================================================================
Routine Description:
    AES-CMAC generate subkey

Arguments:
    Key        Cipher key 128 bits
    KeyLength  The length of Cipher key in bytes

Return Value:
    SubKey1    SubKey 1 128 bits
    SubKey2    SubKey 2 128 bits

Note:
    Reference to RFC 4493
    
    Step 1.  L := AES-128(K, const_Zero);
    Step 2.  if MSB(L) is equal to 0
                then    K1 := L << 1;
                else    K1 := (L << 1) XOR const_Rb;
    Step 3.  if MSB(K1) is equal to 0
                then    K2 := K1 << 1;
                else    K2 := (K1 << 1) XOR const_Rb;
    Step 4.  return K1, K2;
========================================================================
*/
VOID AES_CMAC_GenerateSubKey (
    IN UINT8 Key[],
    IN UINT KeyLength,
    OUT UINT8 SubKey1[],
    OUT UINT8 SubKey2[])
{
    UINT8 MSB_L = 0, MSB_K1 = 0, Top_Bit = 0;
    UINT  SubKey1_Length = 0;
    INT   Index = 0;

    if (KeyLength != AES_KEY128_LENGTH) {
    	DBGPRINT(RT_DEBUG_ERROR, ("AES_CMAC_GenerateSubKey: key length is %d bytes, it must be %d bytes(128 bits).\n", 
            KeyLength, AES_KEY128_LENGTH));
        return;
    } /* End of if */

    /* Step 1: L := AES-128(K, const_Zero); */
    SubKey1_Length = 16;
    RT_AES_Encrypt(Const_Zero, sizeof(Const_Zero), Key, KeyLength, SubKey1, &SubKey1_Length);

    /*
     * Step 2.  if MSB(L) is equal to 0
     *           then    K1 := L << 1;
     *           else    K1 := (L << 1) XOR const_Rb;
     */
    MSB_L = SubKey1[0] & 0x80;    
    for(Index = 0; Index < 15; Index++) {
        Top_Bit = (SubKey1[Index + 1] & 0x80)?1:0;
        SubKey1[Index] <<= 1;
        SubKey1[Index] |= Top_Bit;
    }
    SubKey1[15] <<= 1;
    if (MSB_L > 0) {
        for(Index = 0; Index < 16; Index++)
            SubKey1[Index] ^= Const_Rb[Index];
    } /* End of if */

    /*
     * Step 3.  if MSB(K1) is equal to 0
     *           then    K2 := K1 << 1;
     *           else    K2 := (K1 << 1) XOR const_Rb;
     */
    MSB_K1 = SubKey1[0] & 0x80;
    for(Index = 0; Index < 15; Index++) {
        Top_Bit = (SubKey1[Index + 1] & 0x80)?1:0;
        SubKey2[Index] = SubKey1[Index] << 1;
        SubKey2[Index] |= Top_Bit;
    }
    SubKey2[15] = SubKey1[15] << 1;
    if (MSB_K1 > 0) {
        for(Index = 0; Index < 16; Index++)
            SubKey2[Index] ^= Const_Rb[Index];
    } /* End of if */
} /* End of AES_CMAC_GenerateSubKey */


/*
========================================================================
Routine Description:
    AES-CMAC

Arguments:
    PlainText        Plain text
    PlainTextLength  The length of plain text in bytes
    Key              Cipher key, it may be 16, 24, or 32 bytes (128, 192, or 256 bits)
    KeyLength        The length of cipher key in bytes
    MACTextLength    The length of allocated memory spaces in bytes

Return Value:
    MACText       Message authentication code (128-bit string)
    MACTextLength Return the length of Message authentication code in bytes

Note:
    Reference to RFC 4493
========================================================================
*/
VOID AES_CMAC (
    IN UINT8 PlainText[],
    IN UINT PlainTextLength,
    IN UINT8 Key[],
    IN UINT KeyLength,
    OUT UINT8 MACText[],
    INOUT UINT *MACTextLength)
{
    UINT  PlainBlockStart;
    UINT8 X[AES_BLOCK_SIZES], Y[AES_BLOCK_SIZES];
    UINT8 SubKey1[16];
    UINT8 SubKey2[16];
    INT Index;
    UINT X_Length;

    if (*MACTextLength < AES_MAC_LENGTH) {
    	DBGPRINT(RT_DEBUG_ERROR, ("AES_CMAC: MAC text length is less than %d bytes).\n", 
            AES_MAC_LENGTH));
        return;
    } /* End of if */
    if (KeyLength != AES_KEY128_LENGTH) {
    	DBGPRINT(RT_DEBUG_ERROR, ("AES_CMAC: key length is %d bytes, it must be %d bytes(128 bits).\n", 
            KeyLength, AES_KEY128_LENGTH));
        return;
    } /* End of if */

    /* Step 1.  (K1,K2) := Generate_Subkey(K); */
    NdisZeroMemory(SubKey1, 16);
    NdisZeroMemory(SubKey2, 16);   
    AES_CMAC_GenerateSubKey(Key, KeyLength, SubKey1, SubKey2);

    /*   
     * 2. Main algorithm
     *    - Plain text divide into serveral blocks (16 bytes/block)
     *    - If plain text is not divided with no remainder by block, padding size = (block - remainder plain text)
     *    - Execute RT_AES_Encrypt procedure.
     */
    PlainBlockStart = 0;
    NdisMoveMemory(X, Const_Zero, AES_BLOCK_SIZES);
    while ((PlainTextLength - PlainBlockStart) > AES_BLOCK_SIZES)
    {
        for (Index = 0; Index < AES_BLOCK_SIZES; Index++)
                Y[Index] = PlainText[PlainBlockStart + Index]^X[Index];

        X_Length = sizeof(X);
        RT_AES_Encrypt(Y, sizeof(Y) , Key, KeyLength, X, &X_Length);
        PlainBlockStart += ((UINT) AES_BLOCK_SIZES);
    } /* End of while */
    if ((PlainTextLength - PlainBlockStart) == AES_BLOCK_SIZES) {
        for (Index = 0; Index < AES_BLOCK_SIZES; Index++)
                Y[Index] = PlainText[PlainBlockStart + Index]^X[Index]^SubKey1[Index];        
    } else {    
        NdisZeroMemory(Y, AES_BLOCK_SIZES);
        NdisMoveMemory(Y, &PlainText[PlainBlockStart], (PlainTextLength - PlainBlockStart));
        Y[(PlainTextLength - PlainBlockStart)] = 0x80;
        for (Index = 0; Index < AES_BLOCK_SIZES; Index++)
                Y[Index] = Y[Index]^X[Index]^SubKey2[Index];
    } /* End of if */
    RT_AES_Encrypt(Y, sizeof(Y) , Key, KeyLength, MACText, MACTextLength);
} /* End of AES_CMAC */


